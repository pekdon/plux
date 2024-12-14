#include "line.hh"
#include "script.hh"

#define IS_VAR_CHAR(c) (isalnum((c)) || (c) == '_')

struct ExpandState {
    ExpandState(const plux::ShellEnv& env_, const std::string& shell_)
        : env(env_),
          shell(shell_)
    {
    }

    const plux::ShellEnv& env;
    const std::string& shell;

    std::string exp_line;

    bool in_var = false;
    bool var_curly = false;
    std::string var_name;
};

static void
_expand_var_begin(ExpandState& s, std::string::const_iterator& it,
                  const std::string::const_iterator& last)
{
    if (it == last) {
        // last character
        s.exp_line += '$';
    } else {
        // skip to next character, determines variable type.
        ++it;

        if (*it == '$') {
            s.exp_line += '$';
        } else if (*it == '{') {
            s.in_var = true;
            s.var_curly = true;
            s.var_name = "";
        } else if (IS_VAR_CHAR(*it)) {
            s.in_var = true;
            s.var_curly = false;
            s.var_name = *it;
        } else {
            throw plux::ScriptError(s.shell, "empty variable name");
        }
    }
}

namespace plux
{
    std::string Line::expand_var(const ShellEnv& env, const std::string& shell,
                                 const std::string& line) const
    {
        ExpandState s(env, shell);

        auto it(line.begin());
        auto last(line.end() - 1);
        for (; it != line.end(); ++it) {
            if (s.in_var) {
                if (s.var_curly) {
                    if (*it == '}') {
                        s.in_var = false;
                        append_var_val(env, shell, s.exp_line, s.var_name);
                    } else {
                        s.var_name += *it;
                    }
                } else if (IS_VAR_CHAR(*it)) {
                    s.var_name += *it;
                } else {
                    s.in_var = false;
                    append_var_val(env, shell, s.exp_line, s.var_name);
                    if (*it == '$') {
                        _expand_var_begin(s, it, last);
                    } else {
                        s.exp_line += *it;
                    }
                }
            } else if (*it == '$') {
                _expand_var_begin(s, it, last);
            } else {
                s.exp_line += *it;
            }
        }

        if (s.in_var) {
            if (s.var_curly) {
                throw ScriptError(shell, "end of line while scanning for }");
            }
            append_var_val(env, shell, s.exp_line, s.var_name);
        }

        return s.exp_line;
    }

    void Line::append_var_val(const ShellEnv& env, const std::string& shell,
                              std::string& exp_str,
                              const std::string& var) const
    {
        std::string var_val;
        if (var.empty()) {
            throw ScriptError(shell, "empty variable name");
        } else if (! env.get_env(shell, var, var_val)) {
            throw UndefinedException(shell, "variable", var);
        }
        exp_str += var_val;
    }
}
