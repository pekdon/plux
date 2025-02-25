#pragma once

#include <istream>
#include <memory>
#include <sstream>

#include "plux.hh"
#include "regex.hh"
#include "script.hh"

namespace plux
{
    class ScriptParseCtx {
    public:
        ScriptParseCtx(void)
            : start(0)
        {
        }

        std::string line;
        std::string::size_type start;
        std::string shell;
        std::vector<std::string> process_args;

        bool starts_with(const std::string& str) const {
            if ((str.size() + start) > line.size()) {
                return false;
            }
            return line.compare(start, str.size(), str) == 0;
        }

        bool ends_with(const std::string& str) const {
            if ((str.size() + start) > line.size()) {
                return false;
            }
            return line.compare(line.size() - str.size(),
                                str.size(), str) == 0;
        }

        std::string substr(std::string::size_type trim_start,
                           std::string::size_type trim_end) const {
            if ((trim_start + trim_end + start) >= line.size()) {
                return std::string();
            }
            return line.substr(start + trim_start,
                               line.size() - start - trim_start - trim_end);
        }
    };

    /**
     * Script parser state, transitions go in order of apperance.
     */
    enum parse_state {
        PARSE_STATE_BEGIN,
        PARSE_STATE_DOC,
        PARSE_STATE_HEADERS,
        PARSE_STATE_SHELL,
        PARSE_STATE_CLEANUP
    };

    /**
     * PLUX script parser.
     */
    class ScriptParse {
    public:
        ScriptParse(const std::string& path, std::istream* is,
                    ScriptEnv& env);

        std::unique_ptr<Script> parse(void);

    protected:
        void set_is(std::istream* is) { _is = is; }

        bool next_line(ScriptParseCtx& ctx);

        bool parse_shell(const ScriptParseCtx& ctx, std::string& shell_ret);
        bool parse_process(const ScriptParseCtx& ctx, std::string& shell_ret,
                           std::vector<std::string> &args);

        Line* parse_header_cmd(const ScriptParseCtx& ctx, Script* script);
        Line* parse_line_cmd(const ScriptParseCtx& ctx);

        Line* parse_include(const ScriptParseCtx& ctx);
        Line* parse_config(const ScriptParseCtx& ctx, Script* script);
        template<typename T>
        Line* parse_config_key_val(const ScriptParseCtx& ctx, size_t start);
        Line* parse_global(const ScriptParseCtx& ctx);
        Line* parse_local(const ScriptParseCtx& ctx);
        Line* parse_var_assign(const ScriptParseCtx& ctx, enum var_scope scope);

        Line* parse_timeout(const ScriptParseCtx& ctx);
        Line* parse_call(const ScriptParseCtx& ctx);
        Line* parse_progress(const ScriptParseCtx& ctx);
        Line* parse_log(const ScriptParseCtx& ctx);

        Function* parse_function(const ScriptParseCtx& ctx);
        Macro* parse_macro(const ScriptParseCtx& ctx);

        void parse_args(const ScriptParseCtx& ctx, std::string::size_type start,
                        std::vector<std::string> &args);

        void parse_error(const std::string& line, const std::string& error);

    private:
        void assert_shell_name(const ScriptParseCtx& ctx,
                               const std::string& name);
        parse_state set_parse_state_shell(ScriptParseCtx& ctx, Script &script);

        /** Path to file being parsed. */
        std::string _path;
        /** Opened input stream for path. */
        std::istream* _is;
        /** Global script environment. */
        ScriptEnv& _env;
        /** Current line number. */
        unsigned int _linenumber;

        /** Regular expression for validating shell names. */
        plux::regex _shell_name_regex;
        /** Display string for allowed characters in shell name. */
        static std::string SHELL_NAME_CHARS;
    };
}
