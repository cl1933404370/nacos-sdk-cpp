#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <cstdarg>
#include <stdint.h>
#include "NacosString.h"
#include "src/thread/Mutex.h"
#include "Properties.h"

#define DETAILED_DEBUG_INFO

// TODO:Line info
#ifndef DETAILED_DEBUG_INFO

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(_MSC_VER)
#define log_print(level, format, ...) Logger::debug_print(level, format, ##__VA_ARGS__)
#define log_debug(format, ...) Logger::debug_debug(format, ##__VA_ARGS__)
#define log_info(format, ...) Logger::debug_info(format, ##__VA_ARGS__)
#define log_warn(format, ...) Logger::debug_warn(format, ##__VA_ARGS__)
#define log_error(format, ...) Logger::debug_error(format, ##__VA_ARGS__)
#else
#define STR(X) #X
#define log_print(level, format, args...) Logger::debug_print(level, format, ##args)
#define log_debug(format, args...) Logger::debug_debug(format, ##args)
#define log_info(format, args...) Logger::debug_info(format, ##args)
#define log_warn(format, args...) Logger::debug_warn(format, ##args)
#define log_error(format, args...) Logger::debug_error(format, ##args)
#endif

#else

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(_MSC_VER)
#define log_print(level, format, ...) Logger::debug_print(level, format, ##__VA_ARGS__)
#define log_debug(format, ...) Logger::debug_debug(format, ##__VA_ARGS__)
#define log_info(format, ...) Logger::debug_info(format, ##__VA_ARGS__)
#define log_warn(format, ...) Logger::debug_warn(format, ##__VA_ARGS__)
#define log_error(format, ...) Logger::debug_error(format, ##__VA_ARGS__)
#else
#define log_print(level, format, args...) Logger::debug_print(level, format, ##args)
#define log_debug(format, args...) Logger::debug_debug(format, ##args)
#define log_info(format, args...) Logger::debug_info(format, ##args)
#define log_warn(format, args...) Logger::debug_warn(format, ##args)
#define log_error(format, args...) Logger::debug_error(format, ##args)
#define STR(X) #X
#define log_print(level, format, args...) Logger::debug_print(level, format, ##args)
#define log_debug(format, args...) Logger::debug_debug(format, ##args)
#define log_info(format, args...) Logger::debug_info(format, ##args)
#define log_warn(format, args...) Logger::debug_warn(format, ##args)
#define log_error(format, args...) Logger::debug_error(format, ##args)
#endif

#endif

namespace nacos
{

    enum LOG_LEVEL
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        NONE
    };

    /**
     * Logger
     * Author: Liu, Hanyu
     * This piece of code is a little bit weird, I have to admit
     * Actually there are 2 stages which need logging:
     * 1. The nacos-client is up but no client service is created, need to log issues when creating client service object
     * 2. client service is created, now need to log issue for the client service
     *
     * stage #1 is called the bootstrap stage
     * stage #2 is called the runtime stage
     *
     * On stage #1, the log will be written to:
     * homedir(~)/nacos/logs/nacos-sdk-cpp.log
     *
     * On stage #2, the path can be configured AT MOST ONCE
     * The path will be implicitly configured when the first client service object is created, so if you don't specify it, the default setting(same as #1) will be used
     */
    class Logger
    {
    private:
        static LOG_LEVEL _CUR_SYS_LOG_LEVEL;
        static NacosString _log_base_dir;
        static NacosString _log_file;
        static int64_t _rotate_size;
        static int64_t _last_rotate_time;
        static FILE *_output_file;

        static Mutex setFileLock;

        static int debug_helper(LOG_LEVEL level, const char *format, va_list args);

        static void initializeLogSystem();

    public:
        static void applyLogSettings(Properties &props);
        static void setRotateSize(int64_t rotateSize);
        static void setBaseDir(const NacosString &baseDir);
        static void setLogLevel(LOG_LEVEL level);

        static int64_t getRotateSize();
        static const NacosString &getBaseDir();
        static LOG_LEVEL getLogLevel();

        // Output string in self-defined log_level
        static int debug_print(LOG_LEVEL level, const char *format, ...);

        static int debug_debug(const char *format, ...);

        static int debug_info(const char *format, ...);

        static int debug_warn(const char *format, ...);

        static int debug_error(const char *format, ...);

        static void Init();
        static void deInit();
    };
} // namespace nacos

#endif