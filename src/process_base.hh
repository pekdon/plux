#pragma once

#include <map>
#include <string>
#include <vector>

#include "log.hh"
#include "plux.hh"
#include "regex.hh"
#include "shell_ctx.hh"
#include "shell_log.hh"

namespace plux
{
    /**
     * Exception thrown in management of shell.
     */
    class ShellException : public PluxException {
    public:
        ShellException(const std::string& shell,
                       const std::string& error);
        virtual ~ShellException(void) = default;

        virtual std::string info(void) const override {
            return _shell + " " + _error;
        }
        virtual std::string to_string(void) const override {
            return std::string("ShellException: ") + _shell + " " + _error;
        }

    private:
        std::string _shell;
        std::string _error;
    };

    /**
     * Base for [shell] and [process]
     */
    class ProcessBase : public ShellCtx {
    public:
        ProcessBase(Log& log,
                    ShellLog* shell_log,
                    ProgressLog& progress_log,
                    const std::string& name,
                    const std::string& command,
                    ShellEnv& shell_env,
                    bool trim_special);
        ProcessBase(const ProcessBase& process) = delete;
        virtual ~ProcessBase() = default;

        bool is_alive() const override { return _is_alive; }
        int exitstatus() const override { return _exitstatus; }

        pid_t pid() const override { return _pid; }
        virtual int wait_pid(bool wait);

        const std::string& name(void) const override { return _name; }
        void progress_log(const std::string& msg) override;
        void progress_log(const std::string& context,
                          const std::string& msg) override;

        const std::string& error_pattern(void) const { return _error_pattern; }
        void set_error_pattern(const std::string& pattern) override {
            _error_pattern = pattern;
            try {
                _error = pattern;
                _error_pattern = pattern;
            } catch (const plux::regex_error& ex) {
                throw ShellException(_name,
                                     std::string("invalid error pattern: ")
                                     + ex.what());
            }
        }

        unsigned int timeout(void) const override { return _timeout_ms; }
        void set_timeout(unsigned int timeout_ms) override {
            _timeout_ms = timeout_ms;
        }

        bool input(const std::string& data) override;
        void output(const char* data, ssize_t size) override;

        line_it line_begin() override { return _lines.begin(); }
        line_it line_end() override { return _lines.end(); }
        void line_consume_until(line_it it) override;

        const std::string& buf() const override {
            if (_buf_matched) {
                return plux::empty_string;
            }
            return _buf;
        }
        void consume_buf() override { _buf_matched = true; }

    protected:
        void set_pid(pid_t pid)
        {
            _pid = pid;
            _pid_str = std::to_string(pid);
        }

        bool stop_pid(bool wait);
        std::string line_trim_special(const std::string& line);
        void log_and_throw_strerror(const std::string& msg);

        /** Application log. */
        Log& _log;
        /** Shell IO log. */
        ShellLog* _shell_log;
        /** Progress log. */
        ProgressLog& _progress_log;
        /** Shell environment/variables. */
        ShellEnv& _shell_env;
        /** Set to false when process exits */
        bool _is_alive;
        /** Exit status of process, from waitpid. */
        int _exitstatus;

    private:
        void match_error(const std::string& line, bool is_line);

        /** Shell name. */
        std::string _name;
        /** Match timeout. */
        unsigned int _timeout_ms;
        /** Command run to initialize the shell. */
        std::string _command;
        /** If true, color escape codes are trimmed from the output making
         *  it easier to write match patterns on pure text.
         */
        bool _trim_special;
        /** Error pattern */
        std::string _error_pattern;
        /** Error pattern, if any line matches signal error. */
        plux::regex _error;

        /** Line buffer */
        std::vector<std::string> _lines;
        /** Output buffer */
        std::string _buf;
        /** Set to true when matching buf, will cause buf not to be
            added to lines on newline. */
        bool _buf_matched;

        /** process pid */
        pid_t _pid;
        /** string representation of process pid */
        std::string _pid_str;
    };
}
