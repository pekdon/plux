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
     * void ShellLog.
     */
    class NullShellLog : public ShellLog {
    public:
        NullShellLog() { }
        virtual ~NullShellLog() { }

        virtual void input(const std::string&) override { }
        virtual void output(const char*, ssize_t) override { }
    };

    /**
     * File backed ShellLog.
     */
    class FileShellLog : public ShellLog {
    public:
        FileShellLog(const std::string& path, const std::string& shell,
                     bool tail);
        virtual ~FileShellLog(void);

        virtual void input(const std::string& data) override;
        virtual void output(const char* data, ssize_t size) override;

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
    };

    /**
     * File backed ProgressLog.
     */
    class FileProgressLog : public ProgressLog
    {
    public:
        explicit FileProgressLog(const std::string& path);
        virtual ~FileProgressLog(void);

        virtual void log(const std::string& shell,
                         const std::string& msg) override;

    private:
        /** Log file */
        std::ofstream _log;
    };

}
