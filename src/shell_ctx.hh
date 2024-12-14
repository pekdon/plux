#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "plux.hh"

namespace plux
{
    /**
     * Environment map, used for OS environment, global and local
     * variables.
     */
    typedef std::map<std::string, std::string> env_map;
    typedef env_map::iterator env_map_it;
    typedef env_map::const_iterator env_map_const_it;

    /**
     * Variable scope.
     */
    enum var_scope {
        /** Function local variable. */
        VAR_SCOPE_FUNCTION,
        /** Variable exists within shell and functions called from
         * shell. */
        VAR_SCOPE_SHELL,
        /** Variabel exists globally during script execution, typically
         * pre-filled with OS environment. */
        VAR_SCOPE_GLOBAL
    };

    /**
     * Exception thrown on undefined/missing variable
     */
    class UndefinedException : public PluxException {
    public:
        UndefinedException(const std::string& shell, const std::string& type,
                           const std::string& name) throw();
        virtual ~UndefinedException(void) throw();

        const std::string& shell(void) const { return _shell; }
        const std::string& type(void) const { return _type; }
        const std::string& name(void) const { return _name; }

        virtual std::string info(void) const override {
            std::ostringstream buf("undefined ");
            buf << _type << " " << _name;
            if (! _shell.empty()) {
                buf << " in shell " << _shell;
            }
            return buf.str();
        }

        virtual std::string to_string(void) const override {
            return std::string("UndefinedException: ") + _shell + " " + _name;
        }

    private:
        std::string _shell;
        std::string _type;
        std::string _name;
    };

    /**
     * Environment for shell.
     */
    class ShellEnv {
    public:
        ShellEnv(void) { }
        virtual ~ShellEnv(void) { }

        virtual bool get_env(const std::string& shell, const std::string& key,
                             std::string& val_ret) const = 0;
        virtual void set_env(const std::string& shell, const std::string& key,
                             enum var_scope scope, const std::string& val) = 0;

        /** Enter a new function scope. */
        virtual void push_function(void) = 0;
        /** Leave a function scope, drop all function scoped variables. */
        virtual void pop_function(void) = 0;

        virtual env_map_const_it os_begin(void) const = 0;
        virtual env_map_const_it os_end(void) const = 0;
    };

    /**
     * Shell context.
     */
    class ShellCtx {
    public:
        typedef std::vector<std::string> line_vector;
        typedef line_vector::iterator line_it;

        explicit ShellCtx(void) { }
        virtual ~ShellCtx(void) { }

        virtual const std::string& name(void) const = 0;
        virtual void progress_log(const std::string& msg) = 0;
        virtual void progress_log(const std::string& context,
                                  const std::string& msg) = 0;

        /** Set error pattern for shell */
        virtual void set_error_pattern(const std::string& pattern) = 0;
        /** Get timeout for shell. */
        virtual unsigned int timeout(void) const = 0;
        /** Set timeout for shell */
        virtual void set_timeout(unsigned int timeout_ms) = 0;

        virtual bool input(const std::string& data) = 0;
        virtual void output(const char* data, ssize_t size) = 0;

        virtual line_it line_begin(void) = 0;
        virtual line_it line_end(void) = 0;
        virtual void line_consume_until(line_it it) = 0;

        virtual const std::string& buf(void) const = 0;
        virtual void consume_buf(void) = 0;
    };

    /**
     * Function context.
     */
    class FunctionCtx {
    public:
        FunctionCtx(void) { }
        virtual ~FunctionCtx(void) { }

        virtual const std::string& name(void) const = 0;
        virtual const std::string& shell(void) const = 0;
    };

    /**
     * Function reference including arguments.
     */
    class FunctionArgs {
    public:
        typedef std::vector<std::string>::const_iterator arg_it;

        FunctionArgs() { }
        explicit FunctionArgs(const std::string& fun)
            : _fun(fun) { }
        FunctionArgs(const std::string& fun,
                     const std::vector<std::string>& args)
            : _fun(fun),
              _args(args) { }
        ~FunctionArgs() { }

        const std::string& fun(void) const { return _fun; }
        arg_it arg_begin(void) const { return _args.begin(); }
        arg_it arg_end(void) const { return _args.end(); }

    private:
        std::string _fun;
        std::vector<std::string> _args;
    };
}
