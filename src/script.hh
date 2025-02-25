#pragma once

#include <iostream>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

#include "shell_ctx.hh"
#include "script_env.hh"

namespace plux
{
    /**
     * Error occurred during script execution such as invalid regex
     * compilation, invalid variable names etc.
     */
    class ScriptError : public PluxException {
    public:
        ScriptError(const std::string& shell, const std::string& error) throw();
        virtual ~ScriptError(void) throw();

        const std::string& shell(void) const { return _shell; }
        const std::string& error(void) const { return _error; }

        virtual std::string info(void) const override {
            return _error;
        }
        virtual std::string to_string(void) const override {
            return "ScriptError: " + _shell + " " + _error;
        }

    private:
        /** Shell error occurred in. */
        std::string _shell;
        /** Error message. */
        std::string _error;
    };

    /**
     * Macro.
     */
    class Macro : Function {
    };

    class VarAssign {
    public:
        VarAssign(const std::string& key, const std::string& val)
            : _key(key),
              _val(val)
        {
        }
        virtual ~VarAssign(void) { }

        const std::string& key(void) const { return _key; }
        const std::string& val(void) const { return _val; }

    private:
        std::string _key;
        std::string _val;
    };

    /**
     * Global variable assignment.
     */
    class LineVarAssignGlobal : public Line,
                                public VarAssign {
    public:
        LineVarAssignGlobal(const std::string& file, unsigned int line,
                            const std::string& shell,
                            const std::string& key, const std::string& val)
            : Line(file, line, shell),
              VarAssign(key, val)
        {
        }
        virtual ~LineVarAssignGlobal(void) { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    };

    class LineProgress : public Line {
    public:
        LineProgress(const std::string& file, unsigned int line,
                     const std::string& shell, const std::string& msg)
            : Line(file, line, shell),
              _msg(msg)
        {
        }
        virtual ~LineProgress(void) { }

        const std::string& msg(void) const { return _msg; }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;

    private:
        std::string _msg;
    };

    class LineLog : public LineProgress {
    public:
        using LineProgress::LineProgress;
        virtual ~LineLog(void) { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    };

    /**
     * Call function.
     */
    class LineCall : public Line {
    public:
        LineCall(const std::string& file,  unsigned int line,
                 const std::string& shell,
                 const std::string& name)
            : Line(file, line, shell),
              _name(name)
        {
        }
        LineCall(const std::string& file, unsigned int line,
                 const std::string& shell,
                 const std::string& name,
                 const std::vector<std::string>& args)
            : Line(file, line, shell),
              _name(name),
              _args(args)
        {
        }
        virtual ~LineCall(void) { }

        const std::string& name(void) const { return _name; }
        size_t num_args(void) const { return _args.size(); }
        const std::string& arg(size_t idx) const {
            if (idx >= _args.size()) {
                return plux::empty_string;
            }
            return _args[idx];
        }
        const std::vector<std::string>::const_iterator args_begin(void) const {
            return _args.begin();
        }
        const std::vector<std::string>::const_iterator args_end(void) const {
            return _args.end();
        }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;

    private:
        /** Function name, supports variable expansion. */
        std::string _name;
        /** Argument vector, supports variable expansion. */
        std::vector<std::string> _args;
    };

    /**
     * Set error pattern.
     */
    class LineSetErrorPattern : public Line {
    public:
        LineSetErrorPattern(const std::string& file, unsigned int line,
                            const std::string& shell,
                            const std::string& pattern)
            : Line(file, line, shell),
              _pattern(pattern)
        {
        }
        ~LineSetErrorPattern(void) { }

        const std::string& pattern(void) const { return _pattern; }
        void set_pattern(const std::string& pattern) { _pattern = pattern; }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;

    private:
        /** Error pattern. */
        std::string _pattern;
    };

    /**
     * Shell variable assignment.
     */
    class LineVarAssignShell : public Line,
                               public VarAssign {
    public:
        LineVarAssignShell(const std::string& file, unsigned int line,
                           const std::string& shell,
                           const std::string& key, const std::string& val)
            : Line(file, line, shell),
              VarAssign(key, val)
        {
        }
        virtual ~LineVarAssignShell(void) { }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;
    };

    /**
     * ! and @ output lines.
     */
    class LineOutput : public Line {
    public:
        LineOutput(const std::string& file, unsigned int line,
                   const std::string& shell, const std::string& output)
            : Line(file, line, shell),
              _output(output)
        {
        }
        virtual ~LineOutput(void) { }

        const std::string& output(void) const { return _output; }
        void set_output(const std::string& output) { _output = output; }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;

    private:
        std::string _output;
    };

    /**
     * Modify match timeout for current shell.
     */
    class LineTimeout : public Line {
    public:
        LineTimeout(const std::string& file, unsigned int line,
                    const std::string& shell, unsigned int timeout_ms)
            : Line(file, line, shell),
              _timeout_ms(timeout_ms)
        {
        }

        unsigned int timeout(void) const {
            return _timeout_ms ? _timeout_ms : plux::default_timeout_ms;
        }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;
        virtual std::string to_string(void) const override;

    protected:
        /** Match timeout. */
        unsigned int _timeout_ms;
    };

    /**
     * Base class for single-line match operators.
     */
    class LineMatch : public Line {
    public:
        LineMatch(const std::string& file, unsigned int line,
                  const std::string& shell, const std::string& pattern)
            : Line(file, line, shell),
              _pattern(pattern)
        {
        }
        virtual ~LineMatch(void) { }

        const std::string& pattern(void) const { return _pattern; }
        void set_pattern(const std::string& pattern) { _pattern = pattern; }

        virtual LineRes run(ShellCtx& ctx, ShellEnv& env) override;

    protected:
        virtual bool match(ShellEnv& env, const std::string& shell,
                           const std::string& line, bool partial) = 0;

    private:
        /** Match pattern (string, regex etc) */
        std::string _pattern;
    };

    /**
     * Exact match line.
     */
    class LineExactMatch : public LineMatch {
    public:
        using LineMatch::LineMatch;
        virtual ~LineExactMatch(void) { }

        virtual std::string to_string(void) const override;

    protected:
        virtual bool match(ShellEnv& env, const std::string& shell,
                           const std::string& line, bool is_line) override;
    };

    /**
     * String match with variable expansion.
     */
    class LineVarMatch : public LineMatch {
    public:
        using LineMatch::LineMatch;
        virtual ~LineVarMatch(void) { }

        virtual std::string to_string(void) const override;

    protected:
        virtual bool match(ShellEnv& env, const std::string& shell,
                           const std::string& line, bool is_line) override;
    };

    /**
     * Regular expression line.
     */
    class LineRegexMatch : public LineMatch {
    public:
        using LineMatch::LineMatch;
        virtual ~LineRegexMatch(void) { }

        virtual std::string to_string(void) const override;
        virtual std::string to_string(ShellEnv& env,
                                      const std::string &shell) const override;

    protected:
        virtual bool match(ShellEnv& env, const std::string& shell,
                           const std::string& line, bool is_line) override ;
    };

    /**
     * Parsed PLUX script.
     */
    class Script {
    public:
        Script(const std::string& file, ScriptEnv& env);
        ~Script();

        const std::string& file(void) const { return _file; }
        ScriptEnv& env(void) const { return _env; }
        const std::string& name(void) const { return _name; }
        const std::string& doc(void) const { return _doc; }
        void set_doc(const std::string& doc) { _doc = doc; }

        bool process_add(const std::string& name,
                         const std::vector<std::string>& args);
        bool process_get(const ShellEnv& env, const std::string& name,
                         std::vector<std::string>& args) const;

        line_it header_begin(void) const { return _headers.begin(); }
        line_it header_end(void) const { return _headers.end(); }
        void header_add(Line* header) { _headers.push_back(header); }

        line_it line_begin(void) const { return _lines.begin(); }
        line_it line_end(void) const { return _lines.end(); }
        void line_add(Line* line) { _lines.push_back(line); }

        line_it cleanup_begin(void) const { return _cleanup_lines.begin(); }
        line_it cleanup_end(void) const { return _cleanup_lines.end(); }
        void cleanup_add(Line* line) { _cleanup_lines.push_back(line); }

    private:
        /** starting point for file. */
        const std::string _file;
        /** global script environment. */
        ScriptEnv& _env;
        /** name of the script (basename - .plux ending) */
        std::string _name;
        /** script documentation header */
        std::string _doc;

        /** script headers, include and config directives */
        line_vector _headers;
        /** shell lines */
        line_vector _lines;
        /** lines for the cleanup section. */
        line_vector _cleanup_lines;
        /** process (shell) name to command arguments. */
        std::map<std::string, std::vector<std::string>> _process_args;
    };
}
