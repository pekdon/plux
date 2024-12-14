#include <iostream>
#include <fstream>

#include "regex.hh"
#include "script.hh"
#include "script_parse.hh"

namespace plux
{
    ScriptError::ScriptError(const std::string& shell,
                             const std::string& error) throw()
        : _shell(shell),
          _error(error)
    {
    }

    ScriptError::~ScriptError(void) throw()
    {
    }

    LineRes LineVarAssignGlobal::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto exp_val = expand_var(env, ctx.name(), val());
        env.set_env("", key(), VAR_SCOPE_GLOBAL, exp_val);
        return LineRes(RES_OK);
    }

    std::string LineVarAssignGlobal::to_string(void) const
    {
        return "LineVarAssignGlobal " + key() + "=" + val();
    }

    LineRes LineProgress::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string progress_msg = expand_var(env, ctx.name(), msg());
        ctx.progress_log(progress_msg);
        return LineRes(RES_OK);
    }

    std::string LineProgress::to_string(void) const
    {
        return "LineProgress " + _msg;
    }

    LineRes LineLog::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string context = file() + ":" + std::to_string(line());
        std::string log_msg = expand_var(env, ctx.name(), msg());
        ctx.progress_log(context, log_msg + plux::COLOR_RESET);
        return LineRes(RES_OK);
    }

    std::string LineLog::to_string(void) const
    {
        return "LineLog " + msg();
    }

    LineRes LineCall::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto fun_name = expand_var(env, ctx.name(), _name);
        std::vector<std::string> args;
        std::transform(_args.begin(), _args.end(), std::back_inserter(args),
                       [this, &ctx, &env](const std::string& arg) {
                           return expand_var(env, ctx.name(), arg);
                       });
        return LineRes(RES_CALL, fun_name, args);
    }

    std::string LineCall::to_string(void) const
    {
        return std::string("LineCall ") + _name;
    }

    LineRes LineSetErrorPattern::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string error_pattern = expand_var(env, ctx.name(), _pattern);
        ctx.set_error_pattern(error_pattern);
        return LineRes(RES_OK);
    }

    std::string LineSetErrorPattern::to_string(void) const
    {
        return "LineSetErrorPattern " + _pattern;
    }

    LineRes LineVarAssignShell::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto exp_val = expand_var(env, ctx.name(), val());
        env.set_env(ctx.name(), key(), VAR_SCOPE_SHELL, exp_val);
        return LineRes(RES_OK);
    }

    std::string LineVarAssignShell::to_string(void) const
    {
        return "LineVarAssignLocal " + key() + "=" + val();
    }

    LineRes LineOutput::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string expanded_output = expand_var(env, shell(), _output);
        ctx.input(expanded_output);
        return LineRes(RES_OK);
    }

    std::string LineOutput::to_string(void) const
    {
        return std::string("LineOutput ") +
            _output.substr(0, _output.size() - 1);
    }

    LineRes LineTimeout::run(ShellCtx& ctx, ShellEnv& env)
    {
        ctx.set_timeout(timeout());
        return LineRes(RES_OK);
    }

    std::string LineTimeout::to_string(void) const
    {
        if (_timeout_ms == 0) {
            return std::string("LineTimeout reset");
        } else {
            return std::string("LineTimeout ")
                + std::to_string(_timeout_ms / 1000);
        }
    }

    LineRes LineMatch::run(ShellCtx& ctx, ShellEnv& env)
    {
        ShellCtx::line_it it(ctx.line_begin());
        for (; it != ctx.line_end(); ++it) {
            if (match(env, ctx.name(), *it, true)) {
                // match on complete line, consume all lines until
                // and including this one.
                ctx.line_consume_until(it + 1);
                return LineRes(RES_OK);
            }
        }

        if (match(env, ctx.name(), ctx.buf(), false)) {
            // match on current buffer (no newline), consume all
            // lines and set current line as already matched
            ctx.line_consume_until(ctx.line_end());
            ctx.consume_buf();
            return LineRes(RES_OK);
        }

        return LineRes(RES_NO_MATCH);
    }

    std::string LineExactMatch::to_string(void) const
    {
        return std::string("LineExactMatch ") + pattern();
    }

    bool LineExactMatch::match(ShellEnv& env, const std::string& shell,
                               const std::string& line, bool is_line)
    {
        return line.find(pattern()) != std::string::npos;
    }

    std::string LineVarMatch::to_string(void) const
    {
        return std::string("LineVarMatch ") + pattern();
    }

    bool LineVarMatch::match(ShellEnv& env, const std::string& shell,
                             const std::string& line, bool is_line)
    {
        std::string exp_pattern = expand_var(env, shell, pattern());
        return line.find(exp_pattern) != std::string::npos;
    }

    std::string LineRegexMatch::to_string(void) const
    {
        return std::string("LineRegexMatch ") + pattern();
    }

    std::string LineRegexMatch::to_string(ShellEnv& env,
                                          const std::string& shell) const
    {
        std::string exp_pattern = expand_var(env, shell, pattern());
        return std::string("?") + exp_pattern;
    }

    bool LineRegexMatch::match(ShellEnv& env, const std::string& shell,
                               const std::string& line, bool is_line)
    {
        std::string exp_pattern = expand_var(env, shell, pattern());

        // pattern has end-of-line anchor $ but the provided line
        // is incomplete, this pattern can not match.
        if (! is_line && exp_pattern[exp_pattern.size() - 1] == '$') {
            return false;
        }

        try {
            plux::regex re(exp_pattern);
            plux::smatch matches;
            if (plux::regex_search(line, matches, re)) {
                for (size_t i = 1; i < matches.size(); i++) {
                    env.set_env(shell, std::to_string(i), VAR_SCOPE_SHELL,
                                matches[i].str());
                }
                return true;
            } else {
                return false;
            }
        } catch (const plux::regex_error& ex) {
            std::string msg("regex failed: ");
            msg += ex.what();
            throw ScriptError(shell, msg);
        }
    }

    Script::Script(const std::string& file, ScriptEnv& env)
        : _file(file),
          _env(env),
          _name(path_basename(file))
    {
    }

    Script::~Script(void)
    {
        for (auto it : _headers) {
            delete it;
        }
        for (auto it : _lines) {
            delete it;
        }
        for (auto it : _cleanup_lines) {
            delete it;
        }
    }
}
