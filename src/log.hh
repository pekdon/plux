#ifndef _LOG_HH_
#define _LOG_HH_

#include <string>
#include <sstream>

namespace plux {

    /**
     * Application log level.
     */
    enum log_level {
        LOG_LEVEL_TRACE,
        LOG_LEVEL_DEBUG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARNING,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_NO
    };

    enum log_level level_from_string(const std::string& name);

    /**
     * Log interface for plux application log.
     *
     * operator<< overloaded for common types and can be used to build
     * messages that would otherwise require a set of string
     * operations.
     *
     * A message start with the source, then comes the message and
     * ends with a level.
     *
     *  log << "SOURCE" << "message" << LOG_LEVEL_INFO;
     */
    class Log {
    public:
        Log(enum log_level level);
        virtual ~Log();

        enum log_level level() const { return _level; }
        void set_level(enum log_level level) { _level = level; }

        void trace(const std::string& src, const std::string& msg);
        void debug(const std::string& src, const std::string& msg);
        void info(const std::string& src, const std::string& msg);
        void warning(const std::string& src, const std::string& msg);
        void error(const std::string& src, const std::string& msg);
        void log(enum log_level level, const std::string& src,
                 const std::string& msg);

        // API used for operator<<
        const std::string& msg_src() const { return _msg_src; }
        void msg_set_src(const std::string& src) { _msg_src = src; };
        std::ostringstream& msg_buf() { return _msg_buf; }
        void msg_clear() {
            _msg_src = std::string();
            _msg_buf = std::ostringstream();
        }

    protected:
        virtual void write(enum log_level level,
                           const std::string& full_msg) = 0;

    private:
        std::string format_timestamp();

    private:
        enum log_level _level;

        std::string _msg_src;
        std::ostringstream _msg_buf;
    };

    Log& operator<<(Log& log, const std::string& msg);
    Log& operator<<(Log& log, const char* msg);
    Log& operator<<(Log& log, int num);
    Log& operator<<(Log& log, enum log_level);

    /**
     * Log to file.
     */
    class LogFile : public Log {
    public:
        LogFile(enum log_level level, const std::string& path);
        virtual ~LogFile();

    protected:
        virtual void write(enum log_level level, const std::string& full_msg);

    private:
        std::string _path;
    };
}

#endif // _LOG_HH_
