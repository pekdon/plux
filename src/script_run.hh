#pragma once

#include "cfg.hh"
#include "log.hh"
#include "plux.hh"
#include "script.hh"
#include "shell.hh"
#include "timeout.hh"

namespace plux
{
    /**
     * Script result information including line reference etc.
     */
    class ScriptResult {
    public:
        /**
         * Default construct, OK result.
         */
        ScriptResult(void)
            : _res(RES_OK),
              _line(nullptr)
        {
        }

        /**
         * Error on Line.
         */
        ScriptResult(const LineRes& res, const Line* line,
                     const std::string& error,
                     const std::vector<std::string>& stack)
            : _res(res),
              _line(line),
              _error(error),
              _stack(stack)
        {
        }

        enum line_status status(void) const { return _res.status(); }
        const std::string& file(void) const {
            return _line ? _line->file() : plux::empty_string;
        }
        unsigned int linenumber(void) const {
            return _line ? _line->line() : 0;
        }
        const std::string& error(void) const { return _error; }

        std::vector<std::string>::const_iterator stack_begin(void) const {
            return _stack.begin();
        }
        std::vector<std::string>::const_iterator stack_end(void) const {
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
    class ShellEnvImpl : public ShellEnv {
    public:
        explicit ShellEnvImpl(const env_map& env);
        virtual ~ShellEnvImpl(void);

        virtual bool get_env(const std::string& shell, const std::string& key,
                             std::string& val_ret) const override;
        virtual void set_env(const std::string& shell, const std::string& key,
                             enum var_scope scope,
                             const std::string& val) override;

        virtual void push_function(void) override;
        virtual void pop_function(void) override;

        virtual env_map_const_it os_begin(void) const override;
        virtual env_map_const_it os_end(void) const override;

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
        virtual ~ScriptFunctionCtx(void) { }

        virtual const std::string& name(void) const override { return _name; }
        virtual const std::string& shell(void) const override { return _shell; }

    private:
        std::string _name;
        std::string _shell;
    };

    class ScriptException : public PluxException {
    public:
        explicit ScriptException(const std::string& error) throw();
        virtual ~ScriptException(void) throw();

        virtual std::string info(void) const override { return _error; }
        virtual std::string to_string(void) const override {
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
        ~ScriptRun(void);

        ScriptResult run(void);
        void stop(void);

    protected:
        ScriptResult run_lines(line_it it, line_it end);
        ScriptResult run_line(Line* line);
        ScriptResult run_function(const FunctionArgs& fargs, const Line* line,
                                  const std::string& shell);
        ScriptResult run_function(const FunctionArgs& fargs,
                                  const std::string& shell,
                                  Function* fun);
        ScriptResult run_include(const Line* line,
                                 const std::string& filename);
        ScriptResult run_set(const Line* line, const FunctionArgs& fargs);

        enum line_status wait_for_input(int timeout_ms);

        ShellCtx* get_or_init_shell(Line* line, const std::string& name);
        Shell* init_shell(const std::string& name);
        ShellLog* init_shell_log(const std::string& name);

        const std::string& shell_name(Line* line);

        std::string current_script_path() const;

        void push_function(Function* fun, const std::string& shell);
        void pop_function(Function* fun, const std::string& shell);

        ScriptResult script_error(const LineRes& res, const Line* line,
                                  std::string info);

    private:
        ScriptResult run(const Script* script);

    private:
        /** If set to true, tail shell output */
        bool _tail;
        /** Configuration */
        Cfg _cfg;
        /** Applicaition log. */
        Log& _log;
        /** Progress log. */
        ProgressLog& _progress_log;

        /** Stop "signal" */
        bool _stop;
        /** Map from shell name to Shell */
        std::map<std::string, Shell*> _shells;
        /** Vector with all open Shell logs. */
        std::vector<ShellLog*> _shell_logs;

        /** Timeout for current command. */
        Timeout _timeout;
        /** Shell environment. */
        ShellEnvImpl _env;
        /** Script environment. */
        ScriptEnv& _script_env;
        /** Script */
        std::vector<const Script*> _scripts;
        /** Script Function Context */
        std::vector<ScriptFunctionCtx> _fun_ctx;

        /** Configured shell hook */
        std::string _shell_hook_init;
    };

}
