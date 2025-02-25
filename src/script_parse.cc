#include <fstream>

#include "regex.hh"
#include "script_parse.hh"
#include "script_header.hh"
#include "str.hh"

namespace plux
{
    /** Textual representation of pattern matching shell names, keep
        in sync with _shell_name_regex. */
    std::string ScriptParse::SHELL_NAME_CHARS = "A-Z, a-z, 0-9, - and _";

    /**
     * Create a new ScriptParse instance, used for parsing a PLUX
     * script with the parse method. Once parse is called this
     * instance is consumed.
     */
    ScriptParse::ScriptParse(const std::string& path, std::istream *is,
                             ScriptEnv& env)
        : _path(path),
          _is(is),
          _env(env),
          _linenumber(0),
          _shell_name_regex("^\\$?[A-Za-z0-9_-]+$")
    {
    }

    /**
     * Parse PLUX script from constructor provided istream.
     */
    std::unique_ptr<Script> ScriptParse::parse(void)
    {
        auto script = std::unique_ptr<Script>(new Script(_path, _env));

        Line* line_cmd;

        std::string buf;
        ScriptParseCtx ctx;
        enum parse_state state = PARSE_STATE_BEGIN;
        while (next_line(ctx)) {
            switch (state) {
            case PARSE_STATE_BEGIN:
                if (ctx.line.compare("[doc]") != 0) {
                    parse_error(ctx.line, "unexpected content, expected [doc]");
                }
                state = PARSE_STATE_DOC;
                buf = "";
                break;
            case PARSE_STATE_DOC:
                if (ctx.line.compare("[enddoc]") == 0) {
                    script->set_doc(buf);
                    state = PARSE_STATE_HEADERS;
                } else if (ctx.starts_with("[")) {
                    parse_error(ctx.line,
                                "unexpected content, expected [enddoc]");
                } else {
                    buf.append(ctx.line);
                }
                break;
            case PARSE_STATE_HEADERS:
                if (parse_shell(ctx, ctx.shell)
                    || parse_process(ctx, ctx.shell, ctx.process_args)) {
                    state = set_parse_state_shell(ctx, *script);
                } else if (ctx.starts_with("[function ")) {
                    auto fun = parse_function(ctx);
                    script->env().fun_set(fun->name(), fun);
                } else if (ctx.starts_with("[macro ")) {
                    auto macro = parse_macro(ctx);
                    delete macro;
                } else {
                    line_cmd = parse_header_cmd(ctx, script.get());
                    if (line_cmd) {
                        script->header_add(line_cmd);
                    }
                }
                break;
            case PARSE_STATE_SHELL:
                if (parse_shell(ctx, ctx.shell)
                    || parse_process(ctx, ctx.shell, ctx.process_args)) {
                    state = set_parse_state_shell(ctx, *script);
                } else if (ctx.starts_with("[cleanup]")) {
                   state = PARSE_STATE_CLEANUP;
                   ctx.shell = "cleanup";
                } else {
                    line_cmd = parse_line_cmd(ctx);
                    if (line_cmd) {
                        script->line_add(line_cmd);
                    }
                }
                break;
            case PARSE_STATE_CLEANUP:
                line_cmd = parse_line_cmd(ctx);
                if (line_cmd) {
                    script->cleanup_add(line_cmd);
                }
                break;
            }
        }

        return script;
    }

    /**
     * Parse [shell name] command.
     *
     * @return true if line is a valid shell line, false if not a shell command.
     */
    bool ScriptParse::parse_shell(const ScriptParseCtx& ctx,
                                  std::string& shell_ret)
    {
        if (! ctx.starts_with("[shell ")) {
            return false;
        }

        if (! ctx.ends_with("]")) {
            parse_error(ctx.line, "shell command does not end with ]");
        }

        std::string name = ctx.substr(7, 1);
        assert_shell_name(ctx, name);

        shell_ret = name;
        return true;
    }

    /**
     * Parse [process name command arg...] command.
     *
     * @return true if line is a valid process line, false if not a process
     *         command.
     */
    bool ScriptParse::parse_process(const ScriptParseCtx& ctx,
                                    std::string& shell_ret,
                                    std::vector<std::string> &args_ret)
    {
        if (! ctx.starts_with("[process ")) {
            return false;
        }

        if (! ctx.ends_with("]")) {
            parse_error(ctx.line, "process command does not end with ]");
        }

        std::string line = ctx.substr(9, 1);

        // parse name
        size_t name_end = plux::str_scan(line, 0, " ");
        if (name_end == std::string::npos) {
            parse_error(ctx.line, "process require both name and command");
        }
        std::string name = line.substr(0, name_end);
        assert_shell_name(ctx, name);

        // parse command
        size_t command_end = plux::str_scan(line, name_end + 1, " ");
        std::vector<std::string> args;
        if (command_end == std::string::npos) {
            // no arguments
            args.push_back(line.substr(name_end + 1,
                                       line.size() - name_end - 1));
        } else {
            args.push_back(line.substr(name_end + 1,
                                       command_end - name_end - 1));
            // parse arguments
            size_t arg_start = command_end + 1;
            size_t arg_end = plux::str_scan(line, arg_start, " ");
            while (arg_end != std::string::npos) {
                args.push_back(line.substr(arg_start, arg_end - arg_start));
                arg_start = arg_end + 1;
                arg_end = plux::str_scan(line, arg_start, " ");
            }

            if (arg_start < line.size()) {
                args.push_back(line.substr(arg_start));
            }
        }

        shell_ret = name;
        args_ret = args;
        return true;
    }

    /**
     * Parse commands valid in script header.
     */
    Line* ScriptParse::parse_header_cmd(const ScriptParseCtx& ctx,
                                        Script* script)
    {
        if (ctx.starts_with("[include ")) {
            return parse_include(ctx);;
        } else if (ctx.starts_with("[config ")) {
            return parse_config(ctx, script);
        } else if (ctx.starts_with("[global ")) {
            return parse_global(ctx);
        }
        parse_error(ctx.line, "unexpected content in headers");
        return nullptr;
    }

    /**
     * Parse commands valid inside of [shell name] and [cleanup] section
     *
     * @return Line
     */
    Line* ScriptParse::parse_line_cmd(const ScriptParseCtx& ctx)
    {
        if (ctx.starts_with("!")) {
            auto output = ctx.substr(1, 0);
            // FIXME: add list of control characters
            if (output != "$_CTRL_C_") {
                output += "\n";
            }
            return new LineOutput(_path, _linenumber, ctx.shell, output);
        } else if (ctx.starts_with("?")) {
            auto match_start = ctx.line.find_first_not_of("?", ctx.start);
            if ((match_start - ctx.start) == 1) {
                return new LineRegexMatch(_path, _linenumber, ctx.shell,
                                          ctx.substr(1, 0));
            } else if ((match_start - ctx.start) == 2) {
                return new LineVarMatch(_path, _linenumber, ctx.shell,
                                        ctx.substr(2, 0));
            } else {
                return new LineExactMatch(_path, _linenumber, ctx.shell,
                                          ctx.substr(3, 0));
            }
        } else if (ctx.starts_with("-")) {
            return new LineSetErrorPattern(_path, _linenumber, ctx.shell,
                                           ctx.substr(1, 0));
        } else if (ctx.starts_with("[") && ctx.ends_with("]")) {
            if (ctx.starts_with("[global ")) {
                return parse_global(ctx);
            } else if (ctx.starts_with("[local ")) {
                return parse_local(ctx);
            } else if (ctx.starts_with("[timeout")) {
                return parse_timeout(ctx);
            } else if (ctx.starts_with("[call ")) {
                return parse_call(ctx);
            } else if (ctx.starts_with("[progress ")) {
                return parse_progress(ctx);
            } else if (ctx.starts_with("[log ")) {
                return parse_log(ctx);
            } else {
                parse_error(ctx.line,
                            "unexpected content, unsupported function");
            }
        } else {
            parse_error(ctx.line, "unexpected content");
        }
        return nullptr;
    }

    Line* ScriptParse::parse_timeout(const ScriptParseCtx& ctx)
    {
        unsigned int timeout_s = 0;
        if (ctx.line[ctx.start + 8] != ']') {
            std::string timeout_str = ctx.substr(9, 1);
            try {
                timeout_s = std::stoul(timeout_str);
            } catch (std::invalid_argument&) {
                throw ScriptParseError(_path, _linenumber, ctx.line,
                                       "invalid timeout, not a valid number");
            }
        }
        return new LineTimeout(_path, _linenumber, ctx.shell,
                               timeout_s * 1000);
    }

    Line* ScriptParse::parse_call(const ScriptParseCtx& ctx)
    {
        auto name_start = ctx.line.find_first_not_of(" \t", ctx.start + 6);
        auto name_end = ctx.line.find_first_of(" \t", name_start);
        if (name_end == std::string::npos) {
            auto name = ctx.line.substr(name_start,
                                        ctx.line.size() - name_start - 1);
            return new LineCall(_path, _linenumber, ctx.shell,
                                name);
        } else {
            auto name = ctx.line.substr(name_start, name_end - name_start);
            std::vector<std::string> args;
            parse_args(ctx, name_end, args);
            return new LineCall(_path, _linenumber, ctx.shell,
                                name, args);
        }
    }

    Line* ScriptParse::parse_progress(const ScriptParseCtx& ctx)
    {
        return new LineProgress(_path, _linenumber, ctx.shell,
                                ctx.substr(10, 1));
    }

    Line* ScriptParse::parse_log(const ScriptParseCtx& ctx)
    {
        return new LineLog(_path, _linenumber, ctx.shell,
                           ctx.substr(5, 1));
    }

    Function* ScriptParse::parse_function(const ScriptParseCtx& ctx)
    {
        if (! ctx.ends_with("]")) {
            parse_error(ctx.line, "function does not end with ]");
        }

        auto name_end = ctx.line.find_first_of(" \t", ctx.start + 10);
        if (name_end == std::string::npos) {
            name_end = ctx.line.size() - 1;
        }
        auto name = ctx.line.substr(ctx.start + 10, name_end - ctx.start - 10);

        std::vector<std::string> args;
        if (ctx.line[name_end] != ']') {
            parse_args(ctx, name_end, args);
        }

        auto fun = new Function(_path, _linenumber, name, args);
        ScriptParseCtx fun_ctx;
        while (next_line(fun_ctx)) {
            if (fun_ctx.starts_with("[endfunction]")) {
                return fun;
            }

            if (! parse_shell(fun_ctx, fun_ctx.shell)) {
                auto line_cmd = parse_line_cmd(fun_ctx);
                fun->line_add(line_cmd);
            }
        }

        delete fun;

        parse_error("", "EOF while scanning for [endfunction]");
        return nullptr;
    }

    Macro* ScriptParse::parse_macro(const ScriptParseCtx& ctx)
    {
        parse_error(ctx.line, "not implemented");
        return nullptr;
    }

    Line* ScriptParse::parse_include(const ScriptParseCtx& ctx)
    {
        std::string file = ctx.substr(9, 1);
        return new HeaderInclude(_path, _linenumber, file);
    }

    Line* ScriptParse::parse_config(const ScriptParseCtx& ctx, Script* script)
    {
        if (ctx.starts_with("[config require ")) {
            return parse_config_key_val<HeaderConfigRequire>(ctx, 16);
        } else if (ctx.starts_with("[config set ")) {
            return parse_config_key_val<HeaderConfigSet>(ctx, 12);
        }
        parse_error(ctx.line, "unexpected content, config");
        return nullptr;
    }

    template<typename T>
    Line* ScriptParse::parse_config_key_val(const ScriptParseCtx& ctx,
                                            size_t start)
    {
        std::string key, val;
        auto val_start = ctx.line.find('=', ctx.start + start);
        if (val_start == std::string::npos) {
            key = ctx.substr(start, 1);
            val = "";
        } else {
            // value set, require specific value
            key = ctx.substr(start, ctx.line.size() - val_start);
            val = ctx.substr(val_start - ctx.start + 1, 1);
        }
        return new T(_path, _linenumber, key, val);
    }

    Line* ScriptParse::parse_global(const ScriptParseCtx& ctx)
    {
        ScriptParseCtx var_ctx(ctx);
        var_ctx.start += 8;
        return parse_var_assign(var_ctx, VAR_SCOPE_GLOBAL);
    }

    Line* ScriptParse::parse_local(const ScriptParseCtx& ctx)
    {
        ScriptParseCtx var_ctx(ctx);
        var_ctx.start += 7;
        return parse_var_assign(var_ctx, VAR_SCOPE_SHELL);
    }

    Line* ScriptParse::parse_var_assign(const ScriptParseCtx& ctx,
                                        enum var_scope scope)
    {
        std::string::size_type val_start = ctx.line.find('=', ctx.start);
        if (val_start == std::string::npos) {
            parse_error(ctx.line, "missing = in variable assignment");
        }
        std::string key = ctx.line.substr(ctx.start, val_start - ctx.start);
        std::string val = ctx.line.substr(val_start + 1,
                                          ctx.line.size()  - val_start - 2);
        if (scope == VAR_SCOPE_GLOBAL) {
            return new LineVarAssignGlobal(_path, _linenumber, ctx.shell,
                                           key, val);
        } else if (scope == VAR_SCOPE_SHELL) {
            return new LineVarAssignShell(_path, _linenumber, ctx.shell,
                                          key, val);
        } else {
            throw ScriptParseError(_path, _linenumber, ctx.line,
                                   "unsupporter variable scope");
        }
    }

    /**
     * Get next input line of relevance for the script parser thus
     * including comment and empty lines.
     */
    bool ScriptParse::next_line(ScriptParseCtx& ctx)
    {
        bool is_good = _is->good();
        std::getline(*_is, ctx.line);
        while (is_good) {
            _linenumber++;

            ctx.start = ctx.line.find_first_not_of(" \t");
            if (ctx.start == std::string::npos
                || ctx.start == ctx.line.size()
                || ctx.starts_with("#")) {
                // whitespace, empty or comment line
                is_good = _is->good();
                std::getline(*_is, ctx.line);
            } else {
                return true;
            }
        }
        return false;
    }

    void ScriptParse::parse_args(const ScriptParseCtx& ctx,
                                 std::string::size_type start,
                                 std::vector<std::string> &args)
    {
        start = ctx.line.find_first_not_of(" \t", start);
        auto end = ctx.line.size() - (ctx.ends_with("]") ? 1 : 0);
        str_split(ctx.line.substr(start, end - start), args);
    }

    /**
     * throw ScriptParseError for the current file/linenumber with
     * error message and context line.
     */
    void ScriptParse::parse_error(const std::string& line,
                                  const std::string& error)
    {
        throw ScriptParseError(_path, _linenumber, line, error);
    }
}

void plux::ScriptParse::assert_shell_name(const ScriptParseCtx& ctx,
                                          const std::string& name)
{
    if (! plux::regex_match(name, _shell_name_regex)
        || name.compare("cleanup") == 0) {
        std::string error("invalid shell name: ");
        error +=  name + ". only " + SHELL_NAME_CHARS + " allowed";
        parse_error(ctx.line, error);
    }
}

plux::parse_state
plux::ScriptParse::set_parse_state_shell(ScriptParseCtx& ctx, Script &script)
{
    if (! ctx.process_args.empty()
        && ! script.process_add(ctx.shell, ctx.process_args)) {
        std::string error("duplicate process definition: ");
        error += ctx.shell;
        parse_error(ctx.line, error);
    }
    ctx.process_args.clear();
    return PARSE_STATE_SHELL;
}
