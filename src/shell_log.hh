#ifndef _SHELL_LOG_HH_
#define _SHELL_LOG_HH_

#include <fstream>
#include <string>

namespace plux {

    /**
     * Shell output log, one is created for each shell
     */
    class ShellLog {
    public:
        ShellLog() { }
        virtual ~ShellLog() { }

        virtual void input(const std::string& data) = 0;
        virtual void output(const char* data, ssize_t size) = 0;
    };

    /**
     * File backed ShellLog.
     */
    class FileShellLog : public ShellLog {
    public:
        FileShellLog(const std::string& path);
        virtual ~FileShellLog();

        virtual void input(const std::string& data);
        virtual void output(const char* data, ssize_t size);

    private:
        std::ofstream _input;
        std::ofstream _output;
    };

    /**
     * Common progress log shared between all shells that stores
     * information on [progress], [log], [progress-start] and
     * [progress-stop] messages.
     */
    class ProgressLog {
    public:
        ProgressLog() { }
        virtual ~ProgressLog() { }

        virtual void log(const std::string& shell, const std::string& msg) = 0;
        virtual void progress_start(const std::string& shell, const std::string& msg) = 0;
        virtual void progress_stop(const std::string& shell, const std::string& msg) = 0;
    };

    /**
     * File backed ProgressLog.
     */
    class FileProgressLog : public ProgressLog
    {
    public:
        FileProgressLog(const std::string& path);
        virtual ~FileProgressLog();

        virtual void log(const std::string& shell, const std::string& msg);
        virtual void progress_start(const std::string& shell, const std::string& msg);
        virtual void progress_stop(const std::string& shell, const std::string& msg);
    };

}

#endif // _SHELL_LOG_HH_
