#ifndef __TIME_UTILS_H_
#define __TIME_UTILS_H_
#include <cstddef>
#include <stdint.h>
#if defined(_WIN32) || defined(_MSC_VER)
#include <folly/portability/SysTime.h>
#include <WinSock2.h>
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
    };
} // namespace nacos

#endif
