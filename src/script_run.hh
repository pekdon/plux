#ifndef _SCRIPT_RUN_HH_
#define _SCRIPT_RUN_HH_

#include "cfg.hh"
#include "log.hh"
#include "plux.hh"
#include "script.hh"
#include "shell.hh"
#include "timeout.hh"

namespace plux {

    /**
     * Script result information including line reference etc.
     */
    class ScriptResult {
    public:
        /**
         * Default construct, OK result.
         */
        ScriptResult()
            : _res(RES_OK)
        {
        }

        /**
         * Error on Line.
         */
        ScriptResult(LineRes res, const Line* line,
                     const std::string& error,
                     const std::vector<std::string>& stack)
            : _res(res),
              _line(line),
              _error(error),
              _stack(stack)
        {
        }

        enum line_status status() const { return _res.status(); }
        const std::string& file() const {
            return _line ? _line->file() : plux::empty_string;
        }
        unsigned int linenumber() const { return _line ? _line->line() : 0; }
        const std::string& error() const { return _error; }

        std::vector<std::string>::const_iterator stack_begin() const {
            return _stack.begin();
        }
        std::vector<std::string>::const_iterator stack_end() const {
            return _stack.end();
        }

    private:
        /** Result. */
        LineRes _res;
        /** Line error occurred on, can be nullptr. */
        const Line* _line;
        /** Error message. */
        std::string _error;
        /** Function stack. */
        std::vector<std::string> _stack;
    };

    typedef std::map<std::string, env_map> shell_env_map;

    /**
     * Script environment.
     */
    class ScriptEnv : public ShellEnv {
    public:
        ScriptEnv(const env_map& env);
        virtual ~ScriptEnv();

        virtual bool get_env(const std::string& shell, const std::string& key,
                             std::string& val_ret) const override;
        virtual void set_env(const std::string& shell, const std::string& key,
                             enum var_scope scope,
                             const std::string& val) override;

        virtual void push_function() override;
        virtual void pop_function() override;

        virtual env_map_const_it os_begin() const override;
        virtual env_map_const_it os_end() const override;

    private:
        bool get_env_global(const std::string& key, std::string& val_ret) const;
        bool get_env_shell(const std::string& shell, const std::string& key,
                           std::string& val_ret) const;
        bool get_env_function(const std::string& shell, const std::string& key,
                              std::string& val_ret) const;
        bool get_var(const shell_env_map& env, const std::string& shell,
                     const std::string& key, std::string& val_ret) const;

    private:
        env_map _os_env;
        env_map _global;
        shell_env_map _shell;
        std::vector<shell_env_map> _function;
    };

    class ScriptFunctionCtx : public FunctionCtx {
    public:
        ScriptFunctionCtx(const std::string& name, const std::string& shell)
            : _name(name),
              _shell(shell)
        {
        }
        virtual ~ScriptFunctionCtx() { }

        virtual const std::string& name() const override { return _name; }
        virtual const std::string& shell() const override { return _shell; }

    private:
        std::string _name;
        std::string _shell;
    };

    class ScriptException : public PluxException {
    public:
        ScriptException(const std::string& error)
            : _error(error)
        {
        }
        virtual ~ScriptException() { }

        virtual std::string info() const override { return _error; }
        virtual std::string to_string() const override {
            return "ScriptException: " + _error;
        }

    private:
        std::string _error;
    };

    /**
     * Script execution handler, manages shell and IO for a parsed
     * script.
     */
    class ScriptRun {
    public:
        ScriptRun(Log& log, ProgressLog& progress_log, const env_map& env,
                  const Script* script, bool tail);
        ~ScriptRun();

        ScriptResult run();

    protected:
        ScriptResult run_headers(line_it it, line_it end);
        ScriptResult run_lines(line_it it, line_it end);
        LineRes run_line(Line* line);
        LineRes run_function(Function* fun, const std::string& shell,
                             LineRes::arg_it arg_begin,
                             LineRes::arg_it arg_end);
        enum line_status wait_for_input(int timeout_ms);

        ShellCtx* get_or_init_shell(const std::string& name);
        Shell* init_shell(const std::string& name);
        ShellLog* init_shell_log(const std::string& name);

        const std::string& shell_name(Line* line);

        void push_function(Function* fun, const std::string& shell);
        void pop_function(Function* fun, const std::string& shell);

        ScriptResult script_error(LineRes res, const Line* line,
                                  const std::string& info);

    private:
        /** If set to true, tail shell output */
        bool _tail;
        /** Configuratoin */
        Cfg _cfg;
        /** Applicaition log. */
        Log& _log;
        /** Progress log. */
        ProgressLog& _progress_log;

        /** Map from shell name to Shell */
        std::map<std::string, Shell*> _shells;
        /** Vector with all open Shell logs. */
        std::vector<ShellLog*> _shell_logs;

        /** Timeout for current command. */
        Timeout _timeout;
        /** Script/Shell environment. */
        ScriptEnv _env;
        /** Script */
        const Script* _script;
        /** */
        std::vector<ScriptFunctionCtx> _fun_ctx;
    };

}

#endif // _SCRIPT_RUN_HH_
