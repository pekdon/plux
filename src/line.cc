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

static std::string
re_escape_val(const std::string val)
{
    auto it(val.begin());
    bool in_escape;
    std::string escaped;
    for (; it != val.end(); ++it) {
        if (in_escape) {
            in_escape = false;
            escaped += *it;
        } else if (*it == '\\') {
            in_escape = false;
            escaped += *it;
        } else {
            switch (*it) {
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '?':
            case '*':
            case '|':
            case '.':
                escaped += '\\';
                /* FALLTHROUGH */
            default:
                escaped += *it;
            }
        }
    }
    return escaped;
}

namespace plux
{
    std::string expand_var(const ShellEnv& env, const std::string& shell,
                           const std::string& line)
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

    void append_var_val(const ShellEnv& env, const std::string& shell,
                        std::string& exp_str, const std::string& var)
    {
        if (var.empty()) {
            throw ScriptError(shell, "empty variable name");
        }

        // special ${=VAR} that will cause the variable value to be
        // regular expression safe.
        bool re_escape;
        std::string var_name;
        if (var[0] == '=') {
            var_name = var.substr(1);
            re_escape = true;
        } else {
            var_name = var;
            re_escape = false;
        }
        std::string var_val;
        if (!env.get_env(shell, var_name, var_val)) {
            throw UndefinedException(shell, "variable", var_name);
        }

        if (re_escape) {
            exp_str += re_escape_val(var_val);
        } else {
            exp_str += var_val;
        }
    }
}
