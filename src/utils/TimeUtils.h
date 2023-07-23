#ifndef __TIME_UTILS_H_
#define __TIME_UTILS_H_
#include <cstddef>
#include <stdint.h>
#include <xstring>
#if defined(_WIN32) || defined(_MSC_VER)
#include <folly/portability/SysTime.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Iphlpapi.lib")
#elif defined(__linux__)
#include <sys/time.h>
#endif
namespace nacos
{
    class TimeUtils
    {
    public:
        static int64_t getCurrentTimeInMs();
        static void getCurrentTimeInStruct(struct timeval &tv);
        std::string TimeValueToString(const timeval& tv);
        std::string TimevalToString(const timeval& tv);
        std::string timevalToString(const timeval& tv);
    };
} // namespace nacos

#endif
