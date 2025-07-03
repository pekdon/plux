#include <cstring>
#include <iostream>
#include <sstream>

extern "C" {
#include <sys/wait.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
}

#include "os.hh"
#include "stdlib_builtins.hh"
#include "process.hh"
#include "script.hh"
#include "script_parse.hh"
#include "script_run.hh"

#ifndef INFTIM
#define INFTIM -1
#endif // !INFTIM

namespace plux
{
    ScriptException::ScriptException(const std::string& error) throw()
        : _error(error)
    {
    }

    ScriptException::~ScriptException(void) throw()
    {
    }

    ShellEnvImpl::ShellEnvImpl(const env_map& os_env)
        : _os_env(os_env)
    {
        // override local shell settings, could render PS1 setting inactive
        _os_env["ENV"] = "/dev/null";
        // set PS1, used in scripts to match the prompt in a consistent manner
        _os_env["PS1"] = "SH-PROMPT:";
    }

    ShellEnvImpl::~ShellEnvImpl(void) { }

    bool ShellEnvImpl::get_env(const std::string& shell,
                               const std::string& key,
                               std::string& val_ret) const
    {
        return get_env_function(shell, key, val_ret)
            || get_env_shell(shell, key, val_ret)
            || get_env_global(key, val_ret);
    }

    bool ShellEnvImpl::get_env_global(const std::string& key,
                                      std::string& val_ret) const
    {
        env_map_const_it it;
        if ((it = _global.find(key)) != _global.end()) {
            val_ret = it->second;
            return true;
        }
        if ((it = _os_env.find(key)) != _os_env.end()) {
            val_ret = it->second;
            return true;
        }
        if ((it = plux::default_env.find(key)) != default_env.end()) {
            val_ret = it->second;
            return true;
        }
        return false;
    }

    bool ShellEnvImpl::get_env_shell(const std::string& shell,
                                     const std::string& key,
                                     std::string& val_ret) const
    {
        return get_var(_shell, shell, key, val_ret);
    }

    bool ShellEnvImpl::get_env_function(const std::string& shell,
                                        const std::string& key,
                                        std::string& val_ret) const
    {
        if (_function.empty()) {
            return false;
        }
        if (get_var(_function.back(), shell, key, val_ret)) {
            return true;
        }
        return get_var(_function.back(), "", key, val_ret);
    }

    bool ShellEnvImpl::get_var(const shell_env_map& env,
                               const std::string& shell,
                               const std::string& key,
                               std::string& val_ret) const
    {
        auto s_it = env.find(shell);
        if (s_it != env.end()) {
            auto it(s_it->second.find(key));
            if (it != s_it->second.end()) {
                val_ret = it->second;
                return true;
            }
        }
        return false;
    }

    void ShellEnvImpl::set_env(const std::string& shell,
                               const std::string& key,
                               enum var_scope scope, const std::string& val)
    {
        if (scope == VAR_SCOPE_GLOBAL) {
            _global[key] = val;
        } else if (scope == VAR_SCOPE_SHELL) {
            auto it = _shell.find(shell);
            if (it == _shell.end()) {
                _shell[shell] = env_map();
            }
            _shell[shell][key] = val;
        } else if (scope == VAR_SCOPE_FUNCTION) {
            if (_function.empty()) {
                throw ScriptException("set function scoped variable with "
                                      "no active function");
            }
            _function.back()[shell][key] = val;
        }
    }

    void ShellEnvImpl::push_function(void)
    {
        _function.push_back(shell_env_map());
    }

    void ShellEnvImpl::pop_function(void)
    {
        _function.pop_back();
    }

    void ShellEnvImpl::set_os_env() const
    {
        env_map_const_it it(os_begin());
        for (; it != os_end(); ++it) {
            setenv(it->first.c_str(), it->second.c_str(), 1 /* overwrite */);
        }
    }

    env_map_const_it ShellEnvImpl::os_begin() const
    {
        return _os_env.begin();
    }
    env_map_const_it ShellEnvImpl::os_end() const
    {
        return _os_env.end();
    }

    /**
     * Initialize scritp runner with log and default environment to be
     * used for shells.
     */
    ScriptRun::ScriptRun(Log& log, ProgressLog& progress_log,
                         const env_map& env, const Script* script, bool tail)
        : _tail(tail),
          _log(log),
          _progress_log(progress_log),
          _stop(false),
          _timeout(plux::default_timeout_ms()),
          _env(env),
          _script_env(script->env())
    {
        _scripts.push_back(script);
    }

    /**
     * Cleanup resources, stops all shells started during run.
     */
    ScriptRun::~ScriptRun(void)
    {
        stop();

        for (auto it : _shells) {
            delete it.second;
        }
        for (auto it : _shell_logs) {
            delete it;
        }
    }

    /**
     * Run script.
     */
    ScriptResult ScriptRun::run(void)
    {
        const Script* script = _scripts.front();
        return run(script);
    }

    ScriptResult ScriptRun::run(const Script* script)
    {
        auto res = run_lines(script->header_begin(), script->header_end());
        if (res.status() == RES_OK) {
            res = run_lines(script->line_begin(), script->line_end());
            run_lines(script->cleanup_begin(), script->cleanup_end());
        }
        return res;
    }

    /**
     * Set stop signal on script.
     */
    void ScriptRun::stop(void)
    {
        if (! _stop) {
            for (auto it : _shells) {
                it.second->stop();
            }
            _stop = true;
        }
    }

    /**
     * Run script lines, part of ordinary script or cleanup.
     */
    ScriptResult ScriptRun::run_lines(line_it it, line_it end)
    {
        for (; it != end; ++it) {
            try {
                ScriptResult res = run_line(*it);
                if (res.status() != RES_OK) {
                    return res;
                }
            } catch (const PluxException& ex) {
                std::string info = ex.info();
                return script_error(LineRes(RES_ERROR), *it, info);
            }
        }
        return ScriptResult();
    }

    /**
     * Run single script line with timeout, including waiting for more
     * input data.
     */
    ScriptResult ScriptRun::run_line(Line* line)
    {
        if (_stop) {
            throw ScriptException("stopped");
        }

        auto line_shell_name = shell_name(_env, line);
        _log << "ScriptRun" << "run_line " << line_shell_name << " "
             << line->to_string() << LOG_LEVEL_DEBUG;

        ShellCtx* shell = get_or_init_shell(line, line_shell_name);
        _timeout.set_timeout_ms(shell->timeout());
        _timeout.restart();

        LineRes lres(RES_OK);
        lres = line->run(*shell, _env);
        while (lres.status() == RES_NO_MATCH) {
            int timeout_ms = _timeout.get_ms_until_timeout();
            if (timeout_ms == 0) {
                lres = LineRes(RES_TIMEOUT);
            } else {
                lres = LineRes(wait_for_input(timeout_ms));
                if (lres.status() == RES_OK) {
                    lres = line->run(*shell, _env);
                }
            }
        }

        if (lres == RES_CALL) {
            return run_function(lres.fargs(), line, line_shell_name);
        } else if (lres == RES_INCLUDE) {
            return run_include(line, lres.fargs().fun());
        } else if (lres == RES_SET) {
            return run_set(line, lres.fargs());
        } else if (lres != RES_OK) {
            return script_error(lres, line, plux::empty_string,
                                shell);
        } else {
            return ScriptResult();
        }
    }

    ScriptResult ScriptRun::run_function(const FunctionArgs& fargs,
                                         const Line* line,
                                         const std::string& shell)
    {
        auto fun = _script_env.fun_get(fargs.fun());
        if (fun == nullptr) {
            // function not loaded, look for a builting function
            auto it = builtin_funs.find(fargs.fun());
            if (it != builtin_funs.end()) {
                std::string filename = _cfg.stdlib_dir() + "/" + it->second;
                _log << "ScriptRun" << "include builtin " << fargs.fun()
                     << " from " << filename << LOG_LEVEL_TRACE;
                auto res = run_include(line, filename);
                if (res.status() != RES_OK) {
                    return res;
                }
                fun = _script_env.fun_get(fargs.fun());
            }
        }

        if (fun == nullptr) {
            throw UndefinedException(shell, "function", fargs.fun());
        }


        return run_function(fargs, shell, fun);
    }

    ScriptResult ScriptRun::run_function(const FunctionArgs& fargs,
                                         const std::string& shell,
                                         Function* fun)
    {
        _log << "ScriptRun" << "run_function " << fun->name() << " ("
             << fun->num_args() << ")" << LOG_LEVEL_TRACE;

        auto num_args = fargs.arg_end() - fargs.arg_begin();
        if (fun->num_args() != num_args) {
            throw UndefinedException(shell, "argument", fun->name());
        }

        push_function(fun, shell);

        // function arguments provided as global function scoped
        // variables.
        auto arg_name_it = fun->args_begin();
        auto arg_val_it = fargs.arg_begin();
        for (; arg_name_it != fun->args_end(); ++arg_name_it, ++arg_val_it) {
            _env.set_env("", *arg_name_it, VAR_SCOPE_FUNCTION, *arg_val_it);
        }
        _env.set_env("", "FUNCTION_SHELL", VAR_SCOPE_FUNCTION, shell);

        auto it = fun->line_begin();
        for (; it != fun->line_end(); ++it) {
            auto res = run_line(*it);
            if (res.status() != RES_OK) {
                return res;
            }
        }

        pop_function(fun, shell);

        return ScriptResult();
    }

    ScriptResult ScriptRun::run_include(const Line* line,
                                        const std::string& filename)
    {
        _log << "ScriptRun" << "run_include " << filename << LOG_LEVEL_TRACE;

        std::string full_path = path_join(current_script_path(), filename);
        std::filebuf fb;
        if (! fb.open(full_path, std::ios::in)) {
            return script_error(LineRes(RES_ERROR), line,
                                "failed to include: " + filename);
        }

        ScriptResult res;
        std::istream is(&fb);
        try {
            ScriptParse script_parse(filename, &is, _script_env);
            auto script = script_parse.parse();
            res = run(script.get());
        } catch (ScriptParseError& ex) {
            std::ostringstream oss;
            oss << "parsing of " << ex.path() << " failed at line "
                << ex.linenumber() << " "
                << "error: " << ex.error() << " "
                << "content: " << ex.line();
            return script_error(LineRes(RES_ERROR), line, oss.str());
        }
        return res;

    }

    ScriptResult ScriptRun::run_set(const Line* line,
                                    const FunctionArgs& fargs)
    {
        size_t arg_num = fargs.arg_end() - fargs.arg_begin();
        if (arg_num != 2) {
            return script_error(LineRes(RES_ERROR), line, "error");
        }

        auto key_it = fargs.arg_begin();
        auto val_it = fargs.arg_begin() + 1;

        ScriptResult res;
        if (*key_it == "shell_hook_init") {
            if (_shell_hook_init.empty()) {
                _shell_hook_init = *val_it;
            } else {
            }
        } else {
        }
        return res;
    }

    /**
     * Wait for input on all active shells and update their buffers
     * for all shells that have data available.
     */
    enum line_status ScriptRun::wait_for_input(int timeout_ms)
    {
        _log << "ScriptRun" << "wait for input on " << _shells.size()
             << " shells" << LOG_LEVEL_TRACE;

        int num_fds;
        std::unique_ptr<struct pollfd[]> fds(mk_fds(num_fds));

        line_status status = wait_for_input_poll(fds.get(), num_fds,
                                                 timeout_ms);
        if (status != RES_OK) {
            return status;
        }

        auto it = _shells.begin();
        for (size_t i = 0; i < _shells.size(); ++it, i++) {
            if (fds[i].revents & POLLIN) {
                char buf[4096];
                ssize_t nread = read(it->second->fd_input(), buf, sizeof(buf));
                if (nread == -1) {
                    _log << "ScriptRun" << "read failed: " << strerror(errno)
                         << LOG_LEVEL_ERROR;
                    return RES_ERROR;
                } else if (nread == 0) {
                    if (it->second->is_alive()) {
                        _log.debug("ScriptRun", "empty read from alive shell, "
                                   "treat as timeout");
                        return RES_TIMEOUT;
                    }
                    _log.debug("ScriptRun", "empty read from dead shell, "
                               "remove shell");
                    it = _shells.erase(it);
                } else {
                    it->second->output(buf, nread);
                }
            }
        }

        return RES_OK;
    }

    line_status ScriptRun::wait_for_input_poll(struct pollfd *fds, int num_fds,
                                               int timeout_ms)
    {
        handle_signals();
        line_status res = RES_OK;
        while (res == RES_OK) {
            int ret = poll(fds, num_fds, timeout_ms);
            if (ret > 0) {
                return RES_OK;
            }

            if (ret == -1) {
                if (errno == EINTR) {
                    res = handle_signals();
                } else {
                    _log << "ScriptRun" << "poll failed: " << strerror(errno)
                         << LOG_LEVEL_ERROR;
                    res = RES_ERROR;
                }
            } else {
                _log.debug("ScriptRun", "poll timeout");
                res = RES_TIMEOUT;
            }
        }
        return res;
    }

    struct pollfd* ScriptRun::mk_fds(int &num_fds)
    {
        num_fds = _shells.size();
        struct pollfd *fds = new struct pollfd[num_fds];
        auto it = _shells.begin();
        for (int i = 0; it != _shells.end(); ++it) {
            fds[i].fd = it->second->fd_input();
            fds[i].events = POLLIN;
            fds[i].revents = 0;
            i++;
        }
        return fds;
    }

    line_status ScriptRun::handle_signals()
    {
        if (! plux::sigchld) {
            return RES_ERROR;
        }

        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        while (pid > 0) {
            auto it = _shells.begin();
            for (; it != _shells.end(); ++it) {
                if (it->second->pid() == pid) {
                    it->second->set_alive(false, WEXITSTATUS(status));
                    break;
                }
            }
            pid = waitpid(-1, &status, WNOHANG);
        }
        plux::sigchld = false;
        return RES_OK;
    }

    ShellCtx* ScriptRun::get_or_init_shell(Line* line, const std::string& name)
    {
        auto it = _shells.find(name);
        if (it != _shells.end()) {
            return it->second;
        }

        ShellCtx *shell = init_shell(name);
        _shells[name] = shell;
        _log.trace("ScriptRun", "started new shell " + name);

        if (! _shell_hook_init.empty()
            && dynamic_cast<Shell*>(shell) != nullptr) {
            FunctionArgs fargs(_shell_hook_init);
            run_function(fargs, line, name);
        }

        return shell;
    }

    ShellCtx* ScriptRun::init_shell(const std::string& name)
    {
        ShellLog* shell_log = init_shell_log(name);
        std::vector<std::string> args;
        if (_scripts.back()->process_get(_env, name, args)) {
            _log.debug("ScriptRun", "starting new process " + name
                       + " command " + args[0]);
            return new Process(_log, shell_log, _progress_log, name, args,
                               _env);
        } else {
            _log.debug("ScriptRun", "starting new shell " + name);
            return new Shell(_log, shell_log, _progress_log, name, SH, _env);
        }
    }

    ShellLog* ScriptRun::init_shell_log(const std::string& name)
    {
        auto script_name = _scripts.front()->name();
        if (name.size() == 0) {
            _shell_logs.push_back(new NullShellLog());
        } else {
            auto script_path = _cfg.log_dir() + "/" + script_name;
            os_ensure_dir(script_path);
            auto path = script_path + "/" + name;
            _shell_logs.push_back(new FileShellLog(path, name, _tail));
        }
        return _shell_logs.back();
    }

    std::string ScriptRun::shell_name(ShellEnv& env, Line* line)
    {
        if (line->shell().empty()) {
            if (_fun_ctx.empty()) {
                // FIXME: only error if passed headers
                // throw ScriptException("failed to get shell name");
                return plux::empty_string;
            }
            return _fun_ctx.back().shell();
        }
        return line->shell(env, "");
    }

    std::string ScriptRun::current_script_path() const
    {
        return path_dirname(_scripts.back()->file());
    }

    void ScriptRun::push_function(Function* fun, const std::string& shell)
    {
        _fun_ctx.push_back(ScriptFunctionCtx(fun->name(), shell));
        _env.push_function();
    }

    void ScriptRun::pop_function(Function* fun, const std::string& shell)
    {
        _env.pop_function();
        _fun_ctx.pop_back();
    }

    /**
     * Create error script result from provided information,
     * including function call stack.
     */
    ScriptResult ScriptRun::script_error(const LineRes& res, const Line* line,
                                         std::string info, ShellCtx* ctx)
    {
        if (info.empty() && ctx) {
            info = line->to_string(_env, ctx->name());
        }
        if (info.empty()) {
            info = line->to_string();
        }
        if (! res.error().empty()) {
            info += ": ";
            info += res.error();
        }

        std::vector<std::string> stack;
        for (auto it : _fun_ctx) {
            auto fun = _script_env.fun_get(it.name());
            auto frame = fun->file() + ":" + std::to_string(fun->line())
                + " " + it.name();
            stack.push_back(frame);
        }
        return ScriptResult(res, line, info, stack);
    }
}
