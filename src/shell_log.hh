#pragma once

#include <fstream>
#include <string>

namespace plux
{
    /**
     * Shell output log, one is created for each shell
     */
    class ShellLog {
    public:
        ShellLog(void) { }
        virtual ~ShellLog(void) { }

        virtual void input(const std::string& data) = 0;
        virtual void output(const char* data, ssize_t size) = 0;
    };

    /**
     * File backed ShellLog.
     */
    class FileShellLog : public ShellLog {
    public:
        FileShellLog(const std::string& path, const std::string& shell,
                     bool tail);
        virtual ~FileShellLog(void);

        virtual void input(const std::string& data);
        virtual void output(const char* data, ssize_t size);

    private:
        std::string _shell;
        bool _tail;

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
        ProgressLog(void) { }
        virtual ~ProgressLog(void) { }

        virtual void log(const std::string& shell, const std::string& msg) = 0;
        virtual void progress_start(const std::string& shell,
                                    const std::string& msg) = 0;
        virtual void progress_stop(const std::string& shell,
                                   const std::string& msg) = 0;
    };

    /**
     * File backed ProgressLog.
     */
    class FileProgressLog : public ProgressLog
    {
    public:
        FileProgressLog(const std::string& path);
        virtual ~FileProgressLog(void);

        virtual void log(const std::string& shell, const std::string& msg);
        virtual void progress_start(const std::string& shell,
                                    const std::string& msg);
        virtual void progress_stop(const std::string& shell,
                                   const std::string& msg);
    };

}
