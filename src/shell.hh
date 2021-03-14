#ifndef _SHELL_HH_
#define _SHELL_HH_

#include <map>
#include <regex>
#include <string>
#include <vector>

#include "log.hh"
#include "plux.hh"
#include "shell_ctx.hh"
#include "shell_log.hh"

namespace plux {

    /**
     * Exception thrown in management of shell.
     */
    class ShellException : public PluxException {
    public:
        ShellException(const std::string& shell, const std::string& error)
            : _shell(shell),
              _error(error)
        {
        }

        virtual std::string info() const override {
            return _shell + " " + _error;
        }
        virtual std::string to_string() const override {
            return std::string("ShellException: ") + _shell + " " + _error;
        }

    private:
        std::string _shell;
        std::string _error;
    };

    /**
     * Interactive shell
     */
    class Shell : public ShellCtx {
    public:
        typedef std::vector<std::string>::iterator line_it;

        Shell(Log& log,
              ShellLog* shell_log,
              ProgressLog& progress_log,
              const std::string& name,
              const std::string& command,
              ShellEnv& shell_env);
        virtual ~Shell();

        void stop();

        virtual const std::string& name() const override { return _name; }

        const std::string& error_pattern() const { return _error_pattern; }
        virtual void set_error_pattern(const std::string& pattern) override {
            _error_pattern = pattern;
            try {
                _error = std::regex(pattern);
                _error_pattern = pattern;
            } catch (const std::regex_error ex) {
                throw ShellException(_name,
                                     std::string("invalid error pattern: ")
                                     + ex.what());
            }
        }

        virtual unsigned int timeout() const override { return _timeout_ms; }
        virtual void set_timeout(unsigned int timeout_ms) override {
            _timeout_ms = timeout_ms;
        }

        int fd() const { return _fd; }
        pid_t pid() const { return _pid; }

        virtual bool input(const std::string& data) override;
        virtual void output(const char* data, ssize_t size) override;

        virtual line_it line_begin() override { return _lines.begin(); }
        virtual line_it line_end() override { return _lines.end(); }
        virtual void line_consume_until(line_it it) override;

        virtual const std::string& buf() const override {
            if (_buf_matched) {
                return plux::empty_string;
            }
            return _buf;
        }
        virtual void consume_buf() override { _buf_matched = true; }

    private:
        void match_error(const std::string& line, bool is_line);

        void stop_pid(pid_t pid);
        void log_and_throw_strerror(std::string msg);

    private:
        /** Application log. */
        Log& _log;
        /** Shell IO log. */
        ShellLog* _shell_log;
        /** Progress log. */
        ProgressLog& _progress_log;

        /** Shell name. */
        std::string _name;
        /** Match timeout. */
        unsigned int _timeout_ms;
        /** Command run to initialize the shell. */
        std::string _command;
        /** Shell environment/variables. */
        ShellEnv& _shell_env;
        /** Error pattern */
        std::string _error_pattern;
        /** Error pattern, if any line matches signal error. */
        std::basic_regex<char> _error;

        /** Line buffer */
        std::vector<std::string> _lines;
        /** Output buffer */
        std::string _buf;
        /** Set to true when matching buf, will cause buf not to be
            added to lines on newline. */
        bool _buf_matched;

        /** Master fd. */
        int _fd;
        /** Child process pid (shell) */
        pid_t _pid;
    };
};

#endif // _SHELL_HH_
