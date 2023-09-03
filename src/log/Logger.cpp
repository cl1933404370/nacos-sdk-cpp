#include <stdio.h>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || defined(_MSC_VER)
#include <fstream>
#include <io.h>
#include <iostream>
#endif

#include "Logger.h"

#include <ctime>
#include <stdlib.h>
#include <src/thread/PthreadWaraper.h>
#include <sys/stat.h>

#include "NacosExceptions.h"
#include "Properties.h"
#include "constant/ConfigConstant.h"
#include "constant/PropertyKeyConst.h"
#include "src/config/IOUtils.h"
#include "src/utils/ConfigParserUtils.h"
#include "src/utils/DirUtils.h"
#include "src/utils/ParamUtils.h"
#include "src/utils/TimeUtils.h"

namespace nacos
{
    LOG_LEVEL Logger::_CUR_SYS_LOG_LEVEL = ERROR;
    NacosString Logger::_log_base_dir = "";
    NacosString Logger::_log_file = "";
    int64_t Logger::_rotate_size;
    int64_t Logger::_last_rotate_time;
    FILE* Logger::_output_file;
    Mutex Logger::setFileLock;

    // rotate time (in Ms)
    void Logger::setRotateSize(int64_t rotateSize)
    {
        _rotate_size = rotateSize;
    }

    void Logger::setBaseDir(const NacosString& baseDir)
    {
        LockGuard _setFile(setFileLock);
        _log_base_dir = baseDir;
        if (_output_file != nullptr)
        {
            if (const auto error = fclose(_output_file); error != 0)
            {
                perror("fclose");
            }
            _output_file = nullptr;
        }

        _log_file = _log_base_dir + ConfigConstant::FILE_SEPARATOR + "nacos-sdk-cpp.log";
        IOUtils::recursivelyCreate(_log_base_dir);
        if (const errno_t err = fopen_s(&_output_file, _log_file.c_str(), "a"); err != 0)
        {
            throw NacosException(NacosException::UNABLE_TO_OPEN_FILE, _log_file);
        }

        if (_output_file == nullptr)
        {
            NacosString errMsg = "Unable to open file ";
            errMsg += _log_file;
            throw NacosException(NacosException::UNABLE_TO_OPEN_FILE, errMsg);
        }
    }

    void Logger::setLogLevel(LOG_LEVEL level)
    {
        _CUR_SYS_LOG_LEVEL = level;
    };

    int64_t Logger::getRotateSize()
    {
        return _rotate_size;
    }

    const NacosString& Logger::getBaseDir()
    {
        return _log_base_dir;
    }

    LOG_LEVEL Logger::getLogLevel()
    {
        return _CUR_SYS_LOG_LEVEL;
    }

    int Logger::debug_helper(LOG_LEVEL level, char const* const format, const va_list args)
    {
        // Since the current system debug level is greater than this message
        // Suppress it
        if (Logger::_CUR_SYS_LOG_LEVEL > level)
        {
            return 0;
        }
        // va_list argList;

        // va_start(argList, format);
        const int64_t now = TimeUtils::getCurrentTimeInMs();

        struct stat stat_buf;
        stat(_log_file.c_str(), &stat_buf);
        if (stat_buf.st_size >= _rotate_size)
        {
#if defined(__linux__) || defined(__APPLE__)
            truncate(_log_file.c_str(), 0);
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || defined(_MSC_VER)
            if (!_output_file)
            {
                std::ofstream file1(_log_file.c_str());

                if (const errno_t err = fopen_s(&_output_file, _log_file.c_str(), "w"); err != 0)
                {
                    throw NacosException(NacosException::UNABLE_TO_OPEN_FILE, _log_file);
                }
                if (_output_file)
                {
                    _chsize_s(_fileno(_output_file), 0);
                }
            }
#endif
            _last_rotate_time = now;
        }

        const char* log_level;
        switch (level)
        {
        case DEBUG:
            log_level = "[DEBUG]";
            break;
        case INFO:
            log_level = "[INFO]";
            break;
        case WARN:
            log_level = "[WARN]";
            break;
        case ERROR:
            log_level = "[ERROR]";
            break;
        case NONE:
            log_level = "[NONE]";
            break;
        default:
            log_level = "[UNKNOWN]";
            break;
        }

        const time_t t = time(nullptr);
        struct tm currentTime;
        localtime_r(&t, &currentTime);
        // length of [9999-12-31 99:99:99] = 19
        char timeBuf[22];
        if (const auto error = strftime(timeBuf, sizeof(timeBuf), "[%Y-%m-%d %H:%M:%S]", &currentTime); error == 0)
        {
            perror("strftime");
        }

        int retval = fprintf(_output_file, "%s%s", timeBuf, log_level);
        retval += vfprintf(_output_file, format, args);
        if (const auto fflushError = fflush(_output_file); fflushError != 0)
        {
            perror("fflush");
        }
        // va_end(argList);
        return retval;
    }

    // Output string in self-defined log_level
    int Logger::debug_print(LOG_LEVEL level, const char* format, ...)
    {
        va_list argList;

        va_start(argList, format);
        const int retval = debug_helper(level, format, argList);
        va_end(argList);
        return retval;
    }

    int Logger::debug_debug(const char* format, ...)
    {
        va_list argList;

        va_start(argList, format);
        const int retval = debug_helper(DEBUG, format, argList);
        va_end(argList);
        return retval;
    }

    int Logger::debug_info(const char* format, ...)
    {
        va_list argList;

        va_start(argList, format);
        const int retval = debug_helper(INFO, format, argList);
        va_end(argList);
        return retval;
    }

    int Logger::debug_warn(const char* format, ...)
    {
        va_list argList;

        va_start(argList, format);
        const int retval = debug_helper(WARN, format, argList);
        va_end(argList);
        return retval;
    }

    int Logger::debug_error(const char* format, ...)
    {
        va_list argList;

        va_start(argList, format);
        const int retval = debug_helper(ERROR, format, argList);
        va_end(argList);
        return retval;
    }

    void Logger::deInit()
    {
        if (_output_file != nullptr)
        {
            if (const auto fcloseError =  fclose(_output_file); fcloseError != 0)
            {
                perror("fclose");
            }
            _output_file = nullptr;
        }
    }

    void Logger::initializeLogSystem()
    {
        Properties props;

        try
        {
            props = ConfigParserUtils::parseConfigFile(
                DirUtils::getCwd() + ConfigConstant::FILE_SEPARATOR + ConfigConstant::DEFAULT_CONFIG_FILE);
        }
        catch (IOException& e)
        {
            // if we failed to read log settings
            // use default settings as backup
            log_error("Failed to read log settings from %s, using default settings\n", e.what());
        }
        applyLogSettings(props);
    }

    void Logger::applyLogSettings(Properties& props)
    {
        if (!props.contains(PropertyKeyConst::LOG_PATH))
        {
            const NacosString homedir = DirUtils::getHome();
            Logger::setBaseDir(
                homedir + ConfigConstant::FILE_SEPARATOR + "nacos" + ConfigConstant::FILE_SEPARATOR + "logs");
        }
        else
        {
            Logger::setBaseDir(props[PropertyKeyConst::LOG_PATH]);
        }

        if (props.contains(PropertyKeyConst::LOG_LEVEL))
        {
            // default log level is error, if user specifies it within configuration file, update it
            const NacosString& logLevelStr = props[PropertyKeyConst::LOG_LEVEL];

            if (logLevelStr == "DEBUG")
            {
                Logger::setLogLevel(DEBUG);
            }
            else if (logLevelStr == "INFO")
            {
                Logger::setLogLevel(INFO);
            }
            else if (logLevelStr == "WARN")
            {
                Logger::setLogLevel(WARN);
            }
            else if (logLevelStr == "ERROR")
            {
                Logger::setLogLevel(ERROR);
            }
            else if (logLevelStr == "NONE")
            {
                Logger::setLogLevel(NONE);
            }
            else
            {
                throw NacosException(NacosException::INVALID_CONFIG_PARAM,
                                     "Invalid option " + logLevelStr + " for " + PropertyKeyConst::LOG_LEVEL);
            }
        }

        if (!props.contains(PropertyKeyConst::LOG_ROTATE_SIZE))
        {
            Logger::setRotateSize(static_cast<int64_t>(10 * 1024) * 1024); // 10M by default
        }
        else
        {
            const NacosString& logRotateSizeStr = props[PropertyKeyConst::LOG_ROTATE_SIZE];
            if (ParamUtils::isBlank(logRotateSizeStr))
            {
                throw NacosException(NacosException::INVALID_CONFIG_PARAM,
                                     "Invalid option '" + logRotateSizeStr + "' for " +
                                     PropertyKeyConst::LOG_ROTATE_SIZE);
            }

            const size_t logrotateLastch = logRotateSizeStr.length() - 1;
            int mulplier = 1;
            unsigned long logRotateSize;
            switch (logRotateSizeStr[logrotateLastch])
            {
            case 'g':
            case 'G':
                mulplier *= 1024;
            case 'm':
            case 'M':
                mulplier *= 1024;
            case 'k':
            case 'K':
                mulplier *= 1024;
                logRotateSize = strtol(logRotateSizeStr.substr(0, logrotateLastch).c_str(), nullptr, 10);
                // logrotate_lastch = exclude the unit
                logRotateSize *= mulplier;
                break;
            default:
                if (!isdigit(logRotateSizeStr[logrotateLastch]))
                {
                    throw NacosException(NacosException::INVALID_CONFIG_PARAM,
                                         "Invalid option '" + logRotateSizeStr + "' for " +
                                         PropertyKeyConst::LOG_ROTATE_SIZE +
                                         ", the unit of size must be G/g M/m K/k or decimal numbers.");
                }
                logRotateSize = strtol(logRotateSizeStr.substr(0, logrotateLastch).c_str(), nullptr, 10);
                //logRotateSize = atol(logRotateSizeStr.substr(0, logRotateSizeStr.length()).c_str());
                break;
            }

            if (logRotateSize <= 0)
            {
                throw NacosException(NacosException::INVALID_CONFIG_PARAM,
                                     PropertyKeyConst::LOG_ROTATE_SIZE + " should be greater than 0");
            }
            Logger::setRotateSize(logRotateSize);
        }

        log_info("Current log path:%s\n", Logger::getBaseDir().c_str());
    }

    void Logger::Init()
    {
        initializeLogSystem();
    }
} // namespace nacos
