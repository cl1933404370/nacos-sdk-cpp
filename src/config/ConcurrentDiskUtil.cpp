#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <memory>
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
    ConcurrentDiskUtil::getFileContent(const NacosString &file, const NacosString &charsetName) NACOS_THROW(IOException)
    {
        if (IOUtils::checkNotExistOrNotFile(file))
        {
            // TODO:add errorcode
            throw IOException(NacosException::FILE_NOT_FOUND,
                              "checkNotExistOrNotFile failed, unable to access the file, maybe it doesn't exist.");
        }
        size_t toRead = IOUtils::getFileSize(file);
        FILE *fp = fopen(file.c_str(), "rb");
        if (fp == NULL)
        {
            char errbuf[100];
            sprintf(errbuf, "Failed to open file for read, errno: %d", errno);
            // TODO:add errorcode
            throw IOException(NacosException::UNABLE_TO_OPEN_FILE, errbuf);
        }
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        
#else
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
     * @param file        file
     * @param content     content
     * @param charsetName charsetName
     * @return whether write ok
     * @throws IOException IOException
     */
    bool ConcurrentDiskUtil::writeFileContent(
        const NacosString &path,
        const NacosString &content,
        const NacosString &charsetName) NACOS_THROW(IOException)
    {
        FILE *fp = fopen(path.c_str(), "wb");
        if (fp == NULL)
        {
            char errbuf[100];
            sprintf(errbuf, "Failed to open file for write, errno: %d", errno);
            // TODO:add errorcode
            throw IOException(NacosException::UNABLE_TO_OPEN_FILE, errbuf);
        }
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        std::lock_guard<std::mutex> lock(file_mutex);
        std::ofstream file(path);
        if (!file.is_open())
        {
            return false;
        }
        file << content;
        return true;
#else
        flock(fileno(fp), LOCK_SH);
        fwrite(content.c_str(), content.size(), 1, fp);
        flock(fileno(fp), LOCK_UN);
        fclose(fp);
        return true;
#endif
    }
} // namespace nacos