#pragma once

#include <string>
#include <vector>

#include "shell_ctx.hh"

namespace plux
{
    /**
     * Status code from line.
     */
    enum line_status {
        RES_OK,
        RES_ERROR,
        RES_NO_MATCH,
        RES_CALL,
        RES_TIMEOUT,
        RES_INCLUDE,
        RES_SET
    };

    /**
     * Result from line.
     */
    class LineRes {
    public:
        explicit LineRes(enum line_status status)
            : _status(status)
        {
        }
        LineRes(enum line_status status, const std::string& file)
            : _status(status),
              _fargs(file)
        {
        }
        LineRes(enum line_status status,
                const std::string& fun, const std::vector<std::string>& args)
            : _status(status),
              _fargs(fun, args)
        {
        }
        ~LineRes(void) { }

        enum line_status status() const { return _status; }
        const std::string& error() const { return _error; }
        void set_error(const std::string& error) { _error = error; }

        bool operator==(enum line_status status) const {
            return _status == status;
        }
        bool operator!=(enum line_status status) const {
            return _status != status;
        }

        const FunctionArgs& fargs() const { return _fargs; }

    private:
        enum line_status _status;
        std::string _error;
        FunctionArgs _fargs;
    };

    std::string expand_var(const ShellEnv& env, const std::string& shell,
                           const std::string& line);
    void append_var_val(const ShellEnv& env, const std::string& shell,
                        std::string& exp_str, const std::string& var);

    /**
     * Any line in headers or in shell or cleanup.
     */
    class Line {
    public:
        Line(const std::string& file, unsigned int line,
             const std::string& shell)
            : _file(file),
              _line(line),
              _shell(shell)
        {
        }
        virtual ~Line(void) { }

        const std::string& file(void) const { return _file; }
        unsigned int line(void) const { return _line; }
        const std::string& shell() const { return _shell; }
        std::string shell(ShellEnv& env, const std::string& shell) const
        {
            return expand_var(env, shell, _shell);
        }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) = 0;
        virtual std::string to_string(void) const = 0;
        virtual std::string to_string(ShellEnv& env,
                                      const std::string& shell) const
        {
            return plux::empty_string;
        }

    private:
        /** file line was parsed in. */
        const std::string _file;
        /** file line number. */
        unsigned int _line;
        /** shell line applies to, can be empty. */
        std::string _shell;
    };

    typedef std::vector<Line*> line_vector;
    typedef line_vector::const_iterator line_it;
}
