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
#else
#include <unistd.h>
#endif /* _UNISTD_H */

#include "src/log/Logger.h"

namespace nacos{
size_t IOUtils::getFileSize(const NacosString &file) {
    struct stat statbuf;

    if (stat(file.c_str(), &statbuf) == -1) {
        return 0;
    }

    return statbuf.st_size;
}

NacosString IOUtils::readStringFromFile(const NacosString &file, const NacosString &encoding) NACOS_THROW(IOException) {
    size_t toRead = getFileSize(file);
    FILE *fp = fopen(file.c_str(), "rb");
    if (fp == NULL) {
        throw IOException(NacosException::FILE_NOT_FOUND, "File not found:" + file);
    }
    char *buf = new char[toRead + 1];
    fread(buf, toRead, 1, fp);
    buf[toRead] = '\0';
    fclose(fp);
    NacosString result(buf);
    delete[] buf;
    return result;
}

void IOUtils::writeStringToFile(const NacosString &file, const NacosString &data,
                                const NacosString &encoding) NACOS_THROW(IOException) {
    FILE *fp = fopen(file.c_str(), "wb");
    fwrite(data.c_str(), data.size(), 1, fp);
    fclose(fp);
}

//Returns true if:
//a. the file doesn't exist
//b. the file is not a regular file
bool IOUtils::checkNotExistOrNotFile(const NacosString &pathname) {
    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        DWORD attributes = GetFileAttributes(pathname.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            std::cerr << "Error getting file attributes" << std::endl;
            return true;
        }
        return attributes == FILE_ATTRIBUTE_DIRECTORY;
    #else
    struct stat thestat = {0};
    int res = stat(pathname.c_str(), &thestat);

    if (res != 0) {
        if (errno == ENOENT) {
            //a. the file doesn't exist
            return true;
        } else {
            //Maybe something's wrong with the permission
            //Anyway, we have no access to this file
            return true;
        }
    }

    if (!S_ISREG(thestat.st_mode)) {
        //b. the file is not a regular file
        return true;
    } else {

        //This IS a regular file
        return false;
    }
    #endif
}

//Returns true if:
//a. the file doesn't exist
//b. the file is not a directory
bool IOUtils::checkNotExistOrNotDir(const NacosString &pathname) {
    #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
    struct stat info;
    if (stat(pathname.c_str(), &info) != 0) {
        // Failed to get file info, assume it doesn't exist
        return true;
    }
    return !S_ISDIR(info.st_mode);
    #else
    struct stat thestat = {0};
    int res = stat(pathname.c_str(), &thestat);

    if (res != 0) {
        if (errno == ENOENT) {
            //a. the file doesn't exist
            return true;
        } else {
            //Maybe something's wrong with the permission
            //Anyway, we have no access to this file
            return true;
        }
    }

    if (!S_ISDIR(thestat.st_mode)) {
        //b. the file is not a directory
        return true;
    } else {
        //This IS a directory
        return false;
    }
    #endif
}

//TODO:To provide compability across different platforms
NacosString IOUtils::getParentFile(const NacosString &thefile) {
    size_t parentFilePos = thefile.rfind('/');
    //Invalid Directory/Filename, returning empty
    if (parentFilePos == std::string::npos || parentFilePos == 0) {
        return NULLSTR;
    }
    NacosString parentFile = thefile.substr(0, parentFilePos);
    return parentFile;
}

//Upon success, return true
//Upon failure, return false
bool IOUtils::recursivelyRemove(const NacosString &file) {
    struct stat thestat;

    if (stat(file.c_str(), &thestat) == -1 && errno != ENOENT) {
        //Something's wrong, and it's not "FileNotExist", we should record this and exit
        log_error("Failed to stat() file, errno: %d\n", errno);
        return false;
    }

    if (S_ISDIR(thestat.st_mode)) {
        DIR *curdir = opendir(file.c_str());
        struct dirent *direntp = readdir(curdir);
        while (direntp != NULL) {
            if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..")) {
                //skip this dir and parent
                direntp = readdir(curdir);
                continue;
            }
            struct stat subfilestat;
            NacosString subfilepath = file + "/" + direntp->d_name;

            if (stat(subfilepath.c_str(), &subfilestat) == -1 && errno != ENOENT) {
                log_error("Failed to stat() file, errno: %d\n", errno);
                closedir(curdir);
                return false;
            }
            if (S_ISREG(subfilestat.st_mode)) {
                remove(subfilepath.c_str());
            } else if (S_ISDIR(subfilestat.st_mode)) {
                recursivelyRemove(subfilepath);
            }
            //get to the next entry
            direntp = readdir(curdir);
        }
        closedir(curdir);
        remove(file.c_str());
    } else if (S_ISREG(thestat.st_mode)) {
        remove(file.c_str());
    }

    return true;
}

bool IOUtils::cleanDirectory(const NacosString &file) {

#ifdef _WIN32 //|| _WIN64 || _MSC_VER
    std::string search_path = file + "\\*.*";
    WIN32_FIND_DATA file_data;
    HANDLE search_handle = FindFirstFile(search_path.c_str(), &file_data);
    if (search_handle == INVALID_HANDLE_VALUE) {
        return true;
    }
    do {
        if (std::strcmp(file_data.cFileName, ".") != 0
            && std::strcmp(file_data.cFileName, "..") != 0) {

            std::string file_path = file + "\\" + file_data.cFileName;
                if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    SetFileAttributes(file_path.c_str(), FILE_ATTRIBUTE_NORMAL);
                     if (!RemoveDirectory(file_path.c_str())) {
                         SHFILEOPSTRUCT file_op = {};
                        std::string file_path_double_null(file_path + "\0\0"); // Needs to be double-null terminated
                        file_op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
                        file_op.pFrom = file_path_double_null.c_str();
                        file_op.wFunc = FO_DELETE;
                        SHFileOperation(&file_op);
                     }
                }
                else {
                    SetFileAttributes(file_path.c_str(), FILE_ATTRIBUTE_NORMAL);
                    DeleteFile(file_path.c_str());
                }
        }
    } while (FindNextFile(search_handle, &file_data));
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

    DIR *curdir = opendir(file.c_str());
    struct dirent *direntp = readdir(curdir);
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

void IOUtils::recursivelyCreate(const NacosString &file) {
    NacosString parentFile = getParentFile(file);
    if (!isNull(parentFile)) {
        recursivelyCreate(parentFile);
    }

    if (checkNotExistOrNotDir(file)) {
        #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        if (!CreateDirectory(file.c_str(), nullptr)) {
        //todo Œ¥…Ë÷√»®œﬁ
        //    // Get the existing security descriptor
        //    PSECURITY_DESCRIPTOR descriptor = nullptr;
        //    if (GetNamedSecurityInfo(file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, nullptr, &descriptor) != ERROR_SUCCESS) {
        //        std::cerr << "Failed to get security descriptor" << std::endl;
        //    }

        //    // Create a new access control list (ACL)
        //    EXPLICIT_ACCESS access{};
        //    BuildExplicitAccessWithName(&access, const_cast<LPSTR>("Everyone"), FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE, SET_ACCESS, NO_INHERITANCE);

        //    PACL acl = nullptr;
        //    if (SetEntriesInAcl(1, &access, nullptr, &acl) != ERROR_SUCCESS) {
        //        std::cerr << "Failed to create access control list" << std::endl;
        //        return 1;
        //    }

        //    // Set the new security descriptor
        //    if (SetNamedSecurityInfo(const_cast<LPSTR>(file.c_str()), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, acl, nullptr) != ERROR_SUCCESS) {
        //        std::cerr << "Failed to set security descriptor" << std::endl;
        //    }

        //    std::cout << "Permissions set successfully" << std::endl;

        //    std::cerr << "Error creating directory: " << GetLastError() << std::endl;
        //}
        //else {
        //    if (const HANDLE hDir = CreateFile(file.c_str(), GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
        //        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING,
        //        FILE_FLAG_BACKUP_SEMANTICS, nullptr); hDir == INVALID_HANDLE_VALUE) {
        //        std::cerr << "Error opening directory: " << GetLastError() << std::endl;
        //    }
        //    else {
        //        CloseHandle(hDir);
        //    }
        }
        #else
        mkdir(file.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        #endif
    }
}

std::list <NacosString> IOUtils::listFiles(const NacosString &path) {
    struct stat thestat;
    std::list <NacosString> filelist;
    if (stat(path.c_str(), &thestat) == -1 && errno != ENOENT) {
        //Something's wrong, and it's not "FileNotExist", we should record this and exit
        log_error("Failed to stat() file, errno: %d\n", errno);
        return filelist;
    }

    if (!S_ISDIR(thestat.st_mode)) {
        log_error("Call listFiles() on non-directory entity: %s\n", path.c_str());
        return filelist;
    }

    DIR *curdir = opendir(path.c_str());
    struct dirent *direntp = readdir(curdir);
    while (direntp != NULL) {
        if (!strcmp(direntp->d_name, ".") || !strcmp(direntp->d_name, "..")) {
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
}//namespace nacos