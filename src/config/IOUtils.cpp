#include "IOUtils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <list>
#include "dirent.h"
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <process.h>
#include <Aclapi.h>
#include <shellapi.h>
#include <algorithm>
#include <Shlobj.h>
#include <shobjidl_core.h>
#else
#include <unistd.h>
#endif /* _UNISTD_H */


#include "src/log/Logger.h"

namespace nacos
{
    size_t IOUtils::getFileSize(const NacosString& file)
    {
        struct stat statbuf;

        if (stat(file.c_str(), &statbuf) == -1)
        {
            return 0;
        }

        return statbuf.st_size;
    }

    NacosString IOUtils::readStringFromFile(const NacosString& file, const NacosString& encoding)
        NACOS_THROW(IOException) {
        size_t toRead = getFileSize(file);
        FILE* fp = fopen(file.c_str(), "rb");
        if (fp == NULL)
        {
            throw IOException(NacosException::FILE_NOT_FOUND, "File not found:" + file);
        }
        auto buf = new char[toRead + 1];
        fread(buf, toRead, 1, fp);
        buf[toRead] = '\0';
        fclose(fp);
        NacosString result(buf);
        delete[] buf;
        return result;
    }

    void IOUtils::writeStringToFile(const NacosString& file, const NacosString& data,
        const NacosString& encoding) NACOS_THROW(IOException) {
        FILE* fp = fopen(file.c_str(), "wb");
        fwrite(data.c_str(), data.size(), 1, fp);
        fclose(fp);
    }

    //Returns true if:
    //a. the file doesn't exist
    //b. the file is not a regular file
    bool IOUtils::checkNotExistOrNotFile(const NacosString& pathname)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        const DWORD attributes = GetFileAttributes(pathname.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES)
        {
            return true;
        }
        return attributes == FILE_ATTRIBUTE_DIRECTORY;
#else
        struct stat thestat = { 0 };
        int res = stat(pathname.c_str(), &thestat);

        if (res != 0) {
            if (errno == ENOENT) {
                //a. the file doesn't exist
                return true;
            }
            else {
                //Maybe something's wrong with the permission
                //Anyway, we have no access to this file
                return true;
            }
        }

        if (!S_ISREG(thestat.st_mode)) {
            //b. the file is not a regular file
            return true;
        }
        else {

            //This IS a regular file
            return false;
        }
#endif
    }

    //Returns true if:
    //a. the file doesn't exist
    //b. the file is not a directory
    bool IOUtils::checkNotExistOrNotDir(const NacosString& pathname)
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        struct stat info;
        if (stat(pathname.c_str(), &info) != 0)
        {
            // Failed to get file info, assume it doesn't exist
            return true;
        }
        return !S_ISDIR(info.st_mode);
#else
        struct stat thestat = { 0 };
        int res = stat(pathname.c_str(), &thestat);

        if (res != 0) {
            if (errno == ENOENT) {
                //a. the file doesn't exist
                return true;
            }
            else {
                //Maybe something's wrong with the permission
                //Anyway, we have no access to this file
                return true;
            }
        }

        if (!S_ISDIR(thestat.st_mode)) {
            //b. the file is not a directory
            return true;
        }
        else {
            //This IS a directory
            return false;
        }
#endif
    }

    //TODO:To provide compability across different platforms
    NacosString IOUtils::getParentFile(const NacosString& thefile)
    {
        NacosString file = thefile;
#ifdef _WIN32
        std::replace(file.begin(), file.end(), '\\', '/');
        //std::ranges::replace(file, '\\', '/');
#endif
        const size_t parentFilePos = thefile.rfind('/');
        //Invalid Directory/Filename, returning empty
        if (parentFilePos == std::string::npos || parentFilePos == 0)
        {
            return NULLSTR;
        }
        NacosString parentFile = thefile.substr(0, parentFilePos);
        return parentFile;
    }

    //Upon success, return true
    //Upon failure, return false
    bool IOUtils::recursivelyRemove(const NacosString& file)
    {
        struct stat thestat;

        if (stat(file.c_str(), &thestat) == -1 && errno != ENOENT)
        {
            //Something's wrong, and it's not "FileNotExist", we should record this and exit
            log_error("Failed to stat() file, errno: %d\n", errno);
            return false;
        }

        if (S_ISDIR(thestat.st_mode))
        {
            DIR* curdir = opendir(file.c_str());
            struct dirent* direntp = readdir(curdir);
            while (direntp != NULL)
            {
                if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
                {
                    //skip this dir and parent
                    direntp = readdir(curdir);
                    continue;
                }
                struct stat subfilestat;
                NacosString subfilepath = file + "/" + direntp->d_name;

                if (stat(subfilepath.c_str(), &subfilestat) == -1 && errno != ENOENT)
                {
                    log_error("Failed to stat() file, errno: %d\n", errno);
                    closedir(curdir);
                    return false;
                }
                if (S_ISREG(subfilestat.st_mode))
                {
                    remove(subfilepath.c_str());
                }
                else if (S_ISDIR(subfilestat.st_mode))
                {
                    recursivelyRemove(subfilepath);
                }
                //get to the next entry
                direntp = readdir(curdir);
            }
            closedir(curdir);
            remove(file.c_str());
        }
        else if (S_ISREG(thestat.st_mode))
        {
            remove(file.c_str());
        }

        return true;
    }

    bool IOUtils::cleanDirectory(const NacosString& file)
    {
#ifdef _WIN32 //|| _WIN64 || _MSC_VER
        int result = 0;
        const std::string searchPath = file + "\\*.*";
        WIN32_FIND_DATA fileData;
        const HANDLE searchHandle = FindFirstFile(searchPath.c_str(), &fileData);
        if (searchHandle == INVALID_HANDLE_VALUE)
        {
            FindClose(searchHandle);
            return true;
        }
        do
        {
            if (std::strcmp(fileData.cFileName, ".") != 0
                && std::strcmp(fileData.cFileName, "..") != 0)
            {
                std::string file_path = file + "\\" + fileData.cFileName;
                std::replace(file_path.begin(), file_path.end(), '/', '\\');
                //std::ranges::replace(file_path, '/','\\');
                IFileOperation* fileOperation = nullptr;
                CoInitialize(nullptr);
                HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileOperation));
                if (SUCCEEDED(hr))
                {
                    hr = fileOperation->SetOperationFlags(FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI | FOF_ALLOWUNDO);
                    if (SUCCEEDED(hr))
                    {
                        IShellItem* destItem = nullptr;
                        IShellItem* destItem2 = nullptr;
                        hr = SHCreateItemFromParsingName(std::wstring(file_path.begin(), file_path.end()).c_str(), nullptr,
                            IID_PPV_ARGS(&destItem, &destItem2));
                        if (SUCCEEDED(hr))
                        {
                            fileOperation->DeleteItem(destItem, nullptr);
                            hr = fileOperation->PerformOperations();
                            if (!SUCCEEDED(hr))
                            {
                                return false;
                            }

                            destItem->Release();
                        }
                    }
                    fileOperation->Release();
                }
            }
        } while (FindNextFile(searchHandle, &fileData));
        FindClose(searchHandle);
        return true;
#else

        struct stat thestat;

        if (stat(file.c_str(), &thestat) == -1 && errno != ENOENT) {
            //Something's wrong, and it's not "FileNotExist", we should record this and exit
            log_error("Failed to stat() file, errno: %d\n", errno);
            return false;
        }

        if (!S_ISDIR(thestat.st_mode)) {
            log_error("Call cleanDirectory() on non-directory entity: %s\n", file.c_str());
            return false;
        }

        DIR* curdir = opendir(file.c_str());
        struct dirent* direntp = readdir(curdir);
        while (direntp != NULL) {
            if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..")) {
                //skip this dir and parent
                direntp = readdir(curdir);
                continue;
            }
            NacosString subfilepath = file + "/" + direntp->d_name;

            recursivelyRemove(subfilepath);
            //get to the next entry
            direntp = readdir(curdir);
        }
        closedir(curdir);
        return true;
#endif
    }

    void IOUtils::recursivelyCreate(const NacosString& file)
    {
        if (const NacosString parentFile = getParentFile(file); !isNull(parentFile))
            recursivelyCreate(parentFile);

        if (checkNotExistOrNotDir(file))
        {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
            if (!CreateDirectory(file.c_str(), nullptr))
            {
                log_error("Failed to create directory: %s\n", file.c_str());
            }
#else
            mkdir(file.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
        }
    }

    std::list<NacosString> IOUtils::listFiles(const NacosString& path)
    {
        struct stat thestat;
        std::list<NacosString> filelist;
        if (stat(path.c_str(), &thestat) == -1 && errno != ENOENT)
        {
            //Something's wrong, and it's not "FileNotExist", we should record this and exit
            log_error("Failed to stat() file, errno: %d\n", errno);
            return filelist;
        }

        if (!S_ISDIR(thestat.st_mode))
        {
            log_error("Call listFiles() on non-directory entity: %s\n", path.c_str());
            return filelist;
        }

        DIR* curdir = opendir(path.c_str());
        struct dirent* direntp = readdir(curdir);
        while (direntp != NULL)
        {
            if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, ".."))
            {
                //skip this dir and parent
                direntp = readdir(curdir);
                continue;
            }
            NacosString curitem = direntp->d_name;
            filelist.push_back(curitem);
            //get to the next entry
            direntp = readdir(curdir);
        }
        closedir(curdir);

        return filelist;
    }
} //namespace nacos
