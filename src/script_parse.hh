#pragma once

#include <istream>
#include <memory>
#include <sstream>

#include "plux.hh"
#include "regex.hh"
#include "script.hh"

namespace plux
{
    /**
     * Exception thrown if parsing of script fails due to incorrect or
     * incomplete input.
     */
    class ScriptParseError : public PluxException {
    public:
        ScriptParseError(const std::string& path,
                         unsigned int linenumber,
                         const std::string& line,
                         const std::string& error) throw();
        virtual ~ScriptParseError(void) throw();

        const std::string& path(void) const { return _path; }
        unsigned int linenumber(void) const { return _linenumber; }
        const std::string& line(void) const { return _line; }
        const std::string& error(void) const { return _error; }

        virtual std::string info(void) const override { return _error; }
        virtual std::string to_string(void) const override {
            std::ostringstream buf("ScriptParseError: ");
            buf << _path << ":" << _linenumber << " " << _error << std::endl
                << _line;
            return buf.str();
        }

    private:
        /** Path to file parse error occurred in. */
        std::string _path;
        /** Line in file parse error occurred at. */
        unsigned int _linenumber;
        /** Line data parse occured on. */
        std::string _line;
        /** Human readable error message. */
        std::string _error;
    };

    class ScriptParseCtx {
    public:
        ScriptParseCtx(void) { }
        ~ScriptParseCtx(void) { }

        std::string line;
        std::string::size_type start;
        std::string shell;

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

        bool parse_shell(ScriptParseCtx& ctx, std::string& shell_ret);

        Line* parse_header_cmd(const ScriptParseCtx& ctx);
        Line* parse_line_cmd(const ScriptParseCtx& ctx);

        Line* parse_include(const ScriptParseCtx& ctx);
        Line* parse_config(const ScriptParseCtx& ctx);
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
