#include <sys/types.h>

#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <windows.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif /* _UNISTD_H */

#include "src/utils/DirUtils.h"

#if defined(__CYGWIN__) || defined(MS_WINDOWS) || defined(_WIN32) || defined(_WIN64)
#define PATH_MAX 260
#elif defined(linux)
#include <linux/limits.h>
#elif defined(FreeBSD)
#include <sys/syslimits.h>
#else

// we don't know how to handle this situation, check if it's defined
// the is not necessarily an issue, actually in most cases it will just work
#warning "Unknown system or arch, trying fallback strategy. Please check if the compilation is correct"
#ifndef PATH_MAX
#define PATH_MAX 260
#endif

#endif

namespace nacos
{
    NacosString DirUtils::getHome()
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        char homedir[MAX_PATH];
        DWORD len = sizeof(homedir);
        if (GetEnvironmentVariableA("USERPROFILE", homedir, len) == 0)
        {
            throw std::runtime_error("Unable to get home directory");
        }
        return homedir;
#else
        struct passwd *pw = getpwuid(getuid());
        NacosString homedir = pw->pw_dir;
        return homedir;
#endif
    }

    NacosString DirUtils::getCwd()
    {
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        char cwd[MAX_PATH];

        if (GetCurrentDirectoryA(MAX_PATH, cwd) == 0)
        {
            throw std::runtime_error("Unable to get current working directory");
        }

        return cwd;
#else
        char cwd[PATH_MAX];
        NacosString cwds;
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            cwds = cwd;
            return cwds;
        }

        return NULLSTR;
#endif
    }
} // namespace nacos
