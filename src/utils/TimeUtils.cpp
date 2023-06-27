#include "TimeUtils.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Iphlpapi.lib")

namespace nacos
{
    int64_t TimeUtils::getCurrentTimeInMs()
    {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    void TimeUtils::getCurrentTimeInStruct(struct timeval &tv)
    {
        gettimeofday(&tv, nullptr);
    }

} // namespace nacos
