#include "line.hh"
#include "script.hh"

#define IS_VAR_CHAR(c) (isalnum((c)) || (c) == '_')

namespace plux
{
    std::string Line::expand_var(const ShellEnv& env, const std::string& shell,
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

    void Line::append_var_val(const ShellEnv& env, const std::string& shell,
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
}
