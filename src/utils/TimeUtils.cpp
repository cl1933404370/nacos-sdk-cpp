#include "TimeUtils.h"
#include <string>
#include <time.h>
#include <chrono>

namespace nacos
{
    int64_t TimeUtils::getCurrentTimeInMs()
    {
        /* timeval tv{};
         gettimeofday(&tv, nullptr);
         return tv.tv_sec * 1000 + tv.tv_usec / 1000;*/
        const auto now = std::chrono::system_clock::now();
        const auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        const auto value = now_ms.time_since_epoch();
        return value.count();
    }

    void TimeUtils::getCurrentTimeInStruct(struct timeval& tv)
    {
        gettimeofday(&tv, nullptr);
    }

    std::string TimeUtils::TimeValueToString(const timeval& tv)
    {
        time_t nowtime = tv.tv_sec;
        struct tm nowtm{};
#if defined(_WIN32)
        gmtime_s(&nowtm, &nowtime);
#else
    gmtime_r(&nowtime, &nowtm);
#endif

        char buf[50];
        strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", &nowtm);
        uint32_t ret = (uint32_t)(tv.tv_usec + tv.tv_sec * 1000000LL);
        char timeUSecBuf[10];
        sprintf(timeUSecBuf, ".%06d", ret % 1000000);
        std::string s = buf;
        s += timeUSecBuf;
        return s;
    }
} // namespace nacos
