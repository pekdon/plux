#include <iostream>

extern "C" {
#include <poll.h>
#include <unistd.h>
}

#include "script_run.hh"

#ifndef INFTIM
#define INFTIM -1
#endif // !INFTIM

namespace plux {

    ScriptEnv::ScriptEnv(const env_map& os_env)
        : _os_env(os_env)
    {
        _os_env["PS1"] = "SH-PROMPT:";
    }

    ScriptEnv::~ScriptEnv() { }

    bool ScriptEnv::get_env(const std::string& shell, const std::string& key,
                            std::string& val_ret) const
    {
        return get_env_function(shell, key, val_ret)
            || get_env_shell(shell, key, val_ret)
            || get_env_global(key, val_ret);
    }

    bool ScriptEnv::get_env_global(const std::string& key,
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

    bool ScriptEnv::get_env_shell(const std::string& shell,
                                  const std::string& key,
                                  std::string& val_ret) const
    {
        return get_var(_shell, shell, key, val_ret);
    }

    bool ScriptEnv::get_env_function(const std::string& shell,
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

    bool ScriptEnv::get_var(const shell_env_map& env, const std::string& shell,
                            const std::string& key, std::string& val_ret) const
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

    void ScriptEnv::set_env(const std::string& shell, const std::string& key,
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

    void ScriptEnv::push_function()
    {
        _function.push_back(shell_env_map());
    }

    void ScriptEnv::pop_function()
    {
        _function.pop_back();
    }

    env_map_const_it ScriptEnv::os_begin() const { return _os_env.begin(); }
    env_map_const_it ScriptEnv::os_end() const { return _os_env.end(); }

    /**
     * Initialize scritp runner with log and default environment to be
     * used for shells.
     */
    ScriptRun::ScriptRun(Log& log, ProgressLog& progress_log,
                         const env_map& env, const Script* script, bool tail)
        : _tail(tail),
          _log(log),
          _progress_log(progress_log),
          _timeout(default_timeout_ms),
          _env(env),
          _script(script)
    {
    }

    /**
     * Cleanup resources, stops all shells started during run.
     */
    ScriptRun::~ScriptRun()
    {
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
    ScriptResult ScriptRun::run()
    {
        auto res = run_headers(_script->header_begin(), _script->header_end());
        if (res.status() == RES_OK) {
            res = run_lines(_script->line_begin(), _script->line_end());
            run_lines(_script->cleanup_begin(), _script->cleanup_end());
        }
        return res;
    }

    /**
     * Run script headers, include statements, config requirements
     * etc.
     */
    ScriptResult ScriptRun::run_headers(line_it it, line_it end)
    {
        for (; it != end; ++it) {
            try {
                auto lres = run_line(*it);
                if (lres != RES_OK) {
                    return script_error(lres, *it, plux::empty_string);
                }
            } catch (const PluxException& ex) {
                return script_error(RES_ERROR, *it, ex.info());
            }
        }
        return ScriptResult();
    }

    /**
     * Run script lines, part of ordinary script or cleanup.
     */
    ScriptResult ScriptRun::run_lines(line_it it, line_it end)
    {
        for (; it != end; ++it) {
            try {
                auto lres = run_line(*it);
                if (lres != RES_OK) {
                    return script_error(lres, *it, plux::empty_string);
                }
            } catch (const PluxException& ex) {
                return script_error(RES_ERROR, *it, ex.info());
            }
        }
        return ScriptResult();
    }

    /**
     * Run single script line with timeout, including waiting for more
     * input data.
     */
    LineRes ScriptRun::run_line(Line* line)
    {
        auto shell_name = this->shell_name(line);
        _log << "ScriptRun" << "run_line " << shell_name << " "
             << line->to_string() << LOG_LEVEL_DEBUG;

        auto shell = get_or_init_shell(shell_name);
        _timeout.set_timeout_ms(shell->timeout());
        _timeout.restart();

        LineRes res(RES_OK);
        res = line->run(*shell, _env);
        while (res == RES_NO_MATCH) {
            int timeout_ms = _timeout.get_ms_until_timeout();
            if (timeout_ms == 0) {
                res = RES_TIMEOUT;
            } else {
                res = wait_for_input(timeout_ms);
                if (res == RES_OK) {
                    res = line->run(*shell, _env);
                }
            }
        }

        if (res == RES_CALL) {
            auto fun = _script->get_fun(res.fun());
            if (fun == nullptr) {
                throw UndefinedException(shell_name, "function",
                                         res.fun());
            }
            res = run_function(fun, shell_name,
                               res.arg_begin(), res.arg_end());
        }

        return res;
    }

    LineRes ScriptRun::run_function(Function* fun, const std::string& shell,
                                    LineRes::arg_it arg_begin,
                                    LineRes::arg_it arg_end)
    {
        _log << "ScriptRun" << "run_function " << fun->name() << " ("
             << fun->num_args() << ")" << LOG_LEVEL_TRACE;

        auto num_args = arg_end - arg_begin;
        if (fun->num_args() != num_args) {
            std::cout << num_args << std::endl;
            throw UndefinedException(shell, "argument", fun->name());
        }

        push_function(fun, shell);

        // function arguments provided as global function scoped
        // variables.
        auto arg_name_it = fun->args_begin();
        auto arg_val_it = arg_begin;
        for (; arg_name_it != fun->args_end(); ++arg_name_it, ++arg_val_it) {
            _env.set_env("", *arg_name_it, VAR_SCOPE_FUNCTION, *arg_val_it);
        }

        auto res = LineRes(RES_OK);
        auto it = fun->line_begin();
        for (; it != fun->line_end(); ++it) {
            res = run_line(*it);
            if (res != RES_OK) {
                break;
            }
        }

        pop_function(fun, shell);

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
        struct pollfd fds[_shells.size()];
        std::map<std::string, Shell*>::iterator it;

        it = _shells.begin();
        for (int i = 0; it != _shells.end(); ++it, ++i) {
            fds[i].fd = it->second->fd();
            fds[i].events = POLLIN;
            fds[i].revents = 0;
        }

        int ret = poll(fds, _shells.size(), timeout_ms);
        if (ret == 0) {
            _log.debug("ScriptRun", "poll timeout");
            return RES_TIMEOUT;
        } else if (ret == -1) {
            _log << "ScriptRun" << "poll failed: " << strerror(errno)
                 << LOG_LEVEL_ERROR;
            return RES_ERROR;
        }

        it = _shells.begin();
        for (int i = 0; i < _shells.size(); ++it, i++) {
            if (fds[i].revents & POLLIN) {
                char buf[4096];
                ssize_t nread = read(it->second->fd(), buf, sizeof(buf));
                if (nread == -1) {
                    _log << "ScriptRun" << "read failed: " << strerror(errno)
                         << LOG_LEVEL_ERROR;
                    return RES_ERROR;
                } else if (nread == 0) {
                    _log.debug("ScriptRun", "empty read, treat as timeout");
                    return RES_TIMEOUT;
                } else {
                    it->second->output(buf, nread);
                }
            }
        }

        return RES_OK;
    }

    ShellCtx* ScriptRun::get_or_init_shell(const std::string& name)
    {
        auto it = _shells.find(name);
        if (it != _shells.end()) {
            return it->second;
        }

        _log.debug("ScriptRun", "starting new shell " + name);
        auto shell = init_shell(name);
        _shells[name] = shell;
        _log.trace("ScriptRun", "started new shell " + name);

        return shell;
    }

    Shell* ScriptRun::init_shell(const std::string& name)
    {
        ShellLog* shell_log = init_shell_log(name);
        return new Shell(_log, shell_log, _progress_log, name, SH, _env);
    }

    ShellLog* ScriptRun::init_shell_log(const std::string& name)
    {
        auto path = _cfg.log_dir() + "/" + name;
        _shell_logs.push_back(new FileShellLog(path, name, _tail));
        return _shell_logs.back();
    }

    const std::string& ScriptRun::shell_name(Line* line)
    {
        if (line->shell().empty()) {
            if (_fun_ctx.empty()) {
                // FIXME: only error if passed headers
                // throw ScriptException("failed to get shell name");
                return plux::empty_string;
            }
            return _fun_ctx.back().shell();
        }
        return line->shell();
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
    ScriptResult ScriptRun::script_error(LineRes res, const Line* line,
                                         const std::string& info)
    {
        std::vector<std::string> stack;
        for (auto it : _fun_ctx) {
            auto fun = _script->get_fun(it.name());
            auto frame = fun->file() + ":" + std::to_string(fun->line())
                + " " + it.name();
            stack.push_back(frame);
        }
        return ScriptResult(res, line, info, stack);
    }
}
