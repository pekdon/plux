#include <iostream>
#include <fstream>

#include "script.hh"

#define IS_VAR_CHAR(c) (isalnum((c)) || (c) == '_')

namespace plux {

    std::string Line::expand_var(ShellEnv& env, const std::string& shell,
                                 const std::string& line)
    {
        std::string exp_line;

        bool in_var = false;
        bool var_curly = false;
        std::string var_name;

        auto it(line.begin());
        auto last(line.end() - 1);
        for (; it != line.end(); ++it) {
            if (in_var) {
                if (var_curly) {
                    if (*it == '}') {
                        in_var = false;
                        append_var_val(env, shell, exp_line, var_name);
                    } else {
                        var_name += *it;
                    }
                } else if (IS_VAR_CHAR(*it)) {
                    var_name += *it;
                } else {
                    in_var = false;
                    append_var_val(env, shell, exp_line, var_name);
                    exp_line += *it;
                }
            } else if (*it == '$') {
                if (it == last) {
                    // last character
                    exp_line += '$';
                } else {
                    // skip to next character, determines variable
                    // type.
                    ++it;

                    if (*it == '$') {
                        exp_line += '$';
                    } else if (*it == '{') {
                        in_var = true;
                        var_curly = true;
                        var_name = "";
                    } else if (IS_VAR_CHAR(*it)) {
                        in_var = true;
                        var_curly = false;
                        var_name = *it;
                    } else {
                        throw ScriptError(shell, "empty variable name");
                    }
                }
            } else {
                exp_line += *it;
            }
        }

        if (in_var) {
            if (var_curly) {
                throw ScriptError(shell, "end of line while scanning for }");
            }
            append_var_val(env, shell, exp_line, var_name);
        }

        return exp_line;
    }

    void Line::append_var_val(ShellEnv& env, const std::string& shell,
                              std::string& exp_str, const std::string& var)
    {
        std::string var_val;
        if (var.empty()) {
            throw ScriptError(shell, "empty variable name");
        } else if (! env.get_env(shell, var, var_val)) {
            throw UndefinedException(shell, "variable", var);
        }
        exp_str += var_val;
    }

    LineRes HeaderConfigRequire::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string val;
        if (env.get_env("", _key, val)
            && (_val.empty() || _val == val)) {
            return RES_OK;
        }
        return RES_ERROR;
    }

    std::string HeaderConfigRequire::to_string() const
    {
        if (_val.empty()) {
            return "HeaderRequire " + _key;
        } else {
            return "HeaderRequire " + _key + "=" + _val;
        }
    }

    LineRes LineVarAssignGlobal::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto exp_val = expand_var(env, ctx.name(), val());
        env.set_env("", key(), VAR_SCOPE_GLOBAL, exp_val);
        return RES_OK;
    }

    std::string LineVarAssignGlobal::to_string() const
    {
        return "LineVarAssignGlobal " + key() + "=" + val();
    }

    LineRes LineProgress::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::cout << expand_var(env, ctx.name(), msg()) << std::endl;
        return RES_OK;
    }

    std::string LineProgress::to_string() const
    {
        return "LineProgress " + _msg;
    }

    LineRes LineLog::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::cout << expand_var(env, ctx.name(), msg()) << std::endl;
        return RES_OK;
    }

    std::string LineLog::to_string() const
    {
        return "LineLog " + msg();
    }

    LineRes LineCall::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto name = expand_var(env, ctx.name(), _name);
        std::vector<std::string> args;
        for (auto arg : _args) {
            args.push_back(expand_var(env, ctx.name(), arg));
        }
        return LineRes(RES_CALL, name, args);
    }

    std::string LineCall::to_string() const
    {
        return std::string("LineCall ") + _name;
    }

    LineRes LineSetErrorPattern::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string pattern = expand_var(env, ctx.name(), _pattern);
        ctx.set_error_pattern(pattern);
        return RES_OK;
    }

    std::string LineSetErrorPattern::to_string() const
    {
        return "LineSetErrorPattern " + _pattern;
    }

    LineRes LineVarAssignShell::run(ShellCtx& ctx, ShellEnv& env)
    {
        auto exp_val = expand_var(env, ctx.name(), val());
        env.set_env(ctx.name(), key(), VAR_SCOPE_SHELL, exp_val);
        return RES_OK;
    }

    std::string LineVarAssignShell::to_string() const
    {
        return "LineVarAssignLocal " + key() + "=" + val();
    }

    LineRes LineOutput::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string output = expand_var(env, shell(), _output);
        ctx.input(output);
        return RES_OK;
    }

    std::string LineOutput::to_string() const
    {
        return std::string("LineOutput ") +
            _output.substr(0, _output.size() - 1);
    }

    LineRes LineTimeout::run(ShellCtx& ctx, ShellEnv& env)
    {
        ctx.set_timeout(timeout());
        return RES_OK;
    }

    std::string LineTimeout::to_string() const
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

    std::string LineExactMatch::to_string() const
    {
        return std::string("LineExactMatch ") + pattern();
    }

    bool LineExactMatch::match(ShellEnv& env, const std::string& shell,
                               const std::string& line, bool is_line)
    {
        return line.find(pattern()) != std::string::npos;
    }

    std::string LineVarMatch::to_string() const
    {
        return std::string("LineVarMatch ") + pattern();
    }

    bool LineVarMatch::match(ShellEnv& env, const std::string& shell,
                             const std::string& line, bool is_line)
    {
        std::string exp_pattern = expand_var(env, shell, pattern());
        return line.find(exp_pattern) != std::string::npos;
    }

    std::string LineRegexMatch::to_string() const
    {
        return std::string("LineRegexMatch ") + pattern();
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
            std::regex re(exp_pattern);
            std::smatch matches;
            if (std::regex_search(line, matches, re)) {
                for (size_t i = 1; i < matches.size(); i++) {
                    env.set_env(shell, std::to_string(i), VAR_SCOPE_SHELL,
                                matches[i].str());
                }
                return true;
            } else {
                return false;
            }
        } catch (const std::regex_error ex) {
            throw ScriptError(shell, std::string("regex failed: ") + ex.what());
        }
    }

    Script::Script(const std::string& file)
        : _file(file)
    {
    }

    Script::~Script()
    {
        for (auto it : _headers) {
            delete it;
        }
        for (auto it : _funs) {
            delete it.second;
        }
        for (auto it : _lines) {
            delete it;
        }
        for (auto it : _cleanup_lines) {
            delete it;
        }
    }
}
