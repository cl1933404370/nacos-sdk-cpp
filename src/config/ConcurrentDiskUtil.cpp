#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <fstream>
#include <memory>
#include <mutex>
#include <windows.h>
#else
#include <sys/file.h>
#endif

#include "ConcurrentDiskUtil.h"
#include "IOUtils.h"

/**
 * get file content
 *
 * @param file        file
 * @param charsetName charsetName
 * @return content
 * @throws IOException IOException
 */

namespace nacos
{
    std::mutex file_mutex;
    NacosString
    ConcurrentDiskUtil::getFileContent(const NacosString& file, const NacosString& charsetName) NACOS_THROW(IOException)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        const auto asd  = file.c_str();
        const HANDLE fileHandle = CreateFile(TEXT(asd), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            printf("Error opening file: %lu\n", GetLastError());
            return NacosString();
        }
        LARGE_INTEGER fileSize;
        if (!GetFileSizeEx(fileHandle, &fileSize))
        {
            CloseHandle(fileHandle);
            throw std::runtime_error("Failed to get file size");
        }

        DWORD bytesRead = 0;
        constexpr DWORD chunkSize = 8192;
        const auto buffer = std::make_unique<char[]>(chunkSize);
        std::string content;
        while (ReadFile(fileHandle, buffer.get(), chunkSize, &bytesRead, nullptr) && bytesRead > 0)
        {
            content.insert(content.end(), buffer.get(), buffer.get() + bytesRead);
        }
        return content;
#else
        if (IOUtils::checkNotExistOrNotFile(file))
        {
            // TODO:add errorcode
            throw IOException(NacosException::FILE_NOT_FOUND,
                              "checkNotExistOrNotFile failed, unable to access the file, maybe it doesn't exist.");
        }
        size_t toRead = IOUtils::getFileSize(file);
        FILE* fp = fopen(file.c_str(), "rb");
        if (fp == nullptr)
        {
            char errbuf[100];
            sprintf(errbuf, "Failed to open file for read, errno: %d", errno);
            // TODO:add errorcode
            throw IOException(NacosException::UNABLE_TO_OPEN_FILE, errbuf);
        }
        flock(fileno(fp), LOCK_SH);
        char buf[toRead + 1];
        fread(buf, toRead, 1, fp);
        buf[toRead] = '\0';
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return NacosString(buf);
#endif
    }

    /**
     * write file content
     *
     * @param path        file
     * @param content     content
     * @param charsetName charsetName
     * @return whether write ok
     * @throws IOException IOException
     */
    bool ConcurrentDiskUtil::writeFileContent(
        const NacosString& path,
        const NacosString& content,
        const NacosString& charsetName) NACOS_THROW(IOException)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        const HANDLE hFile = CreateFile(TEXT(path.c_str()),
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        nullptr,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            printf("Error opening file: %lu\n", GetLastError());
            return true;
        }

        DWORD dwBytesWritten = 0;
        OVERLAPPED ol;
        ol = {0};
        if (!LockFileEx(hFile, LOCKFILE_EXCLUSIVE_LOCK, 0,
            100, 0, &ol))
        {
            printf("Error locking file: %lu\n", GetLastError());
            CloseHandle(hFile);
            return true;
        }
        if (const DWORD word = static_cast<DWORD>(content.size()); !WriteFile(hFile, content.data(), word, &dwBytesWritten, nullptr))
        {
            printf("Error writing to file: %lu\n", GetLastError());
            UnlockFileEx(hFile, 0, 100, 0, &ol);
            CloseHandle(hFile);
            return true;
        }

        if (!UnlockFileEx(hFile, 0, 100, 0, &ol))
        {
            printf("Error unlocking file: %lu\n", GetLastError());
        }

        CloseHandle(hFile);
        return false;

#else
        FILE* fp = fopen(path.c_str(), "wb");
        if (fp == nullptr)
        {
            char errbuf[100];
            sprintf(errbuf, "Failed to open file for write, errno: %d", errno);
            // TODO:add errorcode
            throw IOException(NacosException::UNABLE_TO_OPEN_FILE, errbuf);
        }
        flock(fileno(fp), LOCK_SH);
        fwrite(content.c_str(), content.size(), 1, fp);
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return true;
#endif
    }
} // namespace nacos
