#include <iostream>
#include <map>

#include "log.hh"
#include "plux.hh"

extern "C" {
#include <time.h>
}

namespace plux {

    /**
     * Map from log level name to log level.
     */
    std::map<std::string, enum log_level,
             case_insensitive_less> name_to_level = {
        {"TRACE", LOG_LEVEL_TRACE},
        {"DEBUG", LOG_LEVEL_DEBUG},
        {"INFO", LOG_LEVEL_INFO},
        {"WARNING", LOG_LEVEL_WARNING},
        {"ERROR", LOG_LEVEL_ERROR}
    };

    /**
     * Map from enum log_level to std::string, keep in sync with
     * enum log_ level
     */
    std::string level_to_name[] = {
        "TRACE  ",
        "DEBUG  ",
        "INFO   ",
        "WARNING",
        "ERROR  ",
        ""
    };

    /**
     * Get log level from string.
     */
    enum log_level level_from_string(const std::string& name)
    {
        auto it = name_to_level.find(name);
        if (it == name_to_level.end()) {
            return LOG_LEVEL_NO;
        }
        return it->second;
    }

    Log::Log(enum log_level level)
        : _level(level)
    {
    }

    Log::~Log()
    {
    }

    void Log::trace(const std::string& src, const std::string& msg)
    {
        log(LOG_LEVEL_TRACE, src, msg);
    }

    void Log::debug(const std::string& src, const std::string& msg)
    {
        log(LOG_LEVEL_DEBUG, src, msg);
    }

    void Log::info(const std::string& src, const std::string& msg)
    {
        log(LOG_LEVEL_INFO, src, msg);
    }

    void Log::warning(const std::string& src, const std::string& msg)
    {
        log(LOG_LEVEL_WARNING, src, msg);
    }

    void Log::error(const std::string& src, const std::string& msg)
    {
        log(LOG_LEVEL_ERROR, src, msg);
    }

    void Log::log(enum log_level level,
                  const std::string& src, const std::string& msg)
    {
        if (level < _level || level >= LOG_LEVEL_NO) {
            return;
        }

        std::string full_msg(format_timestamp());
        full_msg += " ";
        full_msg += level_to_name[level];
        full_msg += " ";
        full_msg += src;
        full_msg += ": ";
        full_msg += msg;
        write(level, full_msg);
    }

    std::string Log::format_timestamp()
    {
        time_t now = time(NULL);
        struct tm tm;
        gmtime_r(&now, &tm);

        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }

    Log& operator<<(Log& log, const std::string& msg)
    {
        return log << msg.c_str();
    }

    Log& operator<<(Log& log, const char* msg)
    {
        if (log.msg_src().size() == 0) {
            log.msg_set_src(msg);
        } else {
            log.msg_buf() << msg;
        }
        return log;
    }

    Log& operator<<(Log& log, int num)
    {
        log.msg_buf() << num;
        return log;
    }

    Log& operator<<(Log& log, enum log_level level)
    {
        log.log(level, log.msg_src(), log.msg_buf().str());
        log.msg_clear();
        return log;
    }

    LogFile::LogFile(enum log_level level, const std::string& path)
        : Log(level),
          _path(path)
    {
    }

    LogFile::~LogFile()
    {
    }

    void LogFile::write(enum log_level level, const std::string& full_msg)
    {
        std::cout << full_msg << std::endl;
    }
};
