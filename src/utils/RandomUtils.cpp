#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <io.h>
#include <process.h>
#include <iostream>
#include <wincrypt.h>
#include <ncrypt.h>
#else
#include <unistd.h>
#endif

#include "src/utils/RandomUtils.h"

namespace nacos
{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#else
    int RandomUtils::fd;
#endif

    ThreadLocal<bool> RandomUtils::initedForThisThread(false);

    void RandomUtils::Init()
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#else
        fd = open("/dev/urandom", O_RDONLY);
#endif
    }

    void RandomUtils::DeInit()
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#else
        if (fd != 0)
        {
            close(fd);
            fd = 0;
        }
#endif
    }

    size_t RandomUtils::getRandomBuffer(void *dest, size_t size)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        HCRYPTPROV hProvider;
        if (!CryptAcquireContext(&hProvider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            throw std::runtime_error("Failed to acquire crypto context");
        }
        if (!CryptGenRandom(hProvider, size, (BYTE*)dest)) {
            throw std::runtime_error("Failed to generate random data");
        }
        CryptReleaseContext(hProvider, 0);
        return size;
#else
        size_t bytes_read = 0;
        while (bytes_read < size)
        {
            bytes_read += read(fd, (char *)dest + bytes_read, size - bytes_read);
        }

        return bytes_read;
#endif
    }

    int RandomUtils::random_inner()
    {

#ifndef _MSC_VER || __WIN32__ || WIN32
#else
        if (!initedForThisThread.get())
        {
            srand(time(NULL));
            initedForThisThread.set(true);
        }
        return rand();
#endif
    }

    int RandomUtils::random(int begin, int end) NACOS_THROW(NacosException)
    {

#ifndef _MSC_VER || __WIN32__ || WIN32
#else
        // sanity check
        if (begin == end || begin > end)
        {
            throw NacosException(NacosException::INVALID_PARAM, "end must be greater than begin");
        }
        long offset = random_inner() % (end - begin + 1);
        return begin + offset;
#endif
    }
} // namespace nacos