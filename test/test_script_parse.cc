#include <iostream>
#include <sstream>

#include "test.hh"
#include "script_parse.hh"

class TestScriptParseCtx : public plux::ScriptParseCtx,
                           public TestSuite {
public:
    TestScriptParseCtx()
        : plux::ScriptParseCtx(),
          TestSuite("ScriptParseCtx")
    {
        register_test("starts_with",
                      std::bind(&TestScriptParseCtx::test_starts_with, this));
        register_test("ends_with",
                      std::bind(&TestScriptParseCtx::test_ends_with, this));
        register_test("substr",
                      std::bind(&TestScriptParseCtx::test_substr, this));
    }

    virtual ~TestScriptParseCtx() { }

    void test_starts_with()
    {
        do_test_starts_with("my full line", 0);
        do_test_starts_with("  my full line", 2);
    }

    void do_test_starts_with(const std::string& line, unsigned int start)
    {
        this->line = line;
        this->start = start;
        ASSERT_EQUAL("starts_with, full line",
                     true, starts_with("my full line"));
        ASSERT_EQUAL("starts_with, prefix",
                     true, starts_with("my "));
        ASSERT_EQUAL("starts_with, other prefix",
                     false, starts_with("their"));
        ASSERT_EQUAL("starts_with, long line",
                     false, starts_with("my full line, long"));
    }

    void test_ends_with()
    {
        do_test_ends_with("my full line", 0);
        do_test_ends_with("  my full line", 2);
    }

    void do_test_ends_with(const std::string& line, unsigned int start)
    {
        this->line = line;
        this->start = start;

        ASSERT_EQUAL("ends_with, full line",
                     true, ends_with("my full line"));
        ASSERT_EQUAL("ends_with, suffix",
                     true, ends_with(" line"));
        ASSERT_EQUAL("ends_with, other suffix",
                     false, ends_with("word"));
        ASSERT_EQUAL("ends_with, long line",
                     false, ends_with("my full line, long"));
    }

    void test_substr()
    {
        do_test_substr("my full line", 0);
        do_test_substr("  my full line", 2);
    }

    void do_test_substr(const std::string& line, unsigned int start)
    {
        this->line = line;
        this->start = start;

        ASSERT_EQUAL("substr, full line", "my full line", substr(0, 0));
        ASSERT_EQUAL("substr, prefix", "my", substr(0, 10));
        ASSERT_EQUAL("substr, suffix", "line", substr(8, 0));
        ASSERT_EQUAL("substr, middle", "full", substr(3, 5));
    }
};

class TestScriptParse : public plux::ScriptParse,
                        public TestSuite {
public:
    TestScriptParse(std::istream* is, plux::ScriptEnv& script_env)
            : plux::ScriptParse(":memory:", is, script_env),
          TestSuite("ScriptParse")
    {
        register_test("next_line",
                      std::bind(&TestScriptParse::test_next_line, this));

        register_test("parse", std::bind(&TestScriptParse::test_parse, this));

        register_test("parse_function",
                      std::bind(&TestScriptParse::test_parse_function, this));

        register_test("parse_header_require",
                      std::bind(&TestScriptParse::test_parse_config, this));
        register_test("parse_shell",
                      std::bind(&TestScriptParse::test_parse_shell, this));
        register_test("parse_call",
                      std::bind(&TestScriptParse::test_parse_line_call, this));
        register_test("parse_line_cmd_output",
                      std::bind(&TestScriptParse::test_parse_line_cmd_output,
                                this));
        register_test("parse_line_cmd_error",
                      std::bind(&TestScriptParse::test_parse_line_cmd_error,
                                this));
        register_test("parse_line_cmd_match",
                      std::bind(&TestScriptParse::test_parse_line_cmd_match,
                                this));
        register_test("parse_line_cmd_global",
                      std::bind(&TestScriptParse::test_parse_line_cmd_global,
                                this));
        register_test("parse_line_cmd_local",
                      std::bind(&TestScriptParse::test_parse_line_cmd_local,
                                this));
        register_test("parse_line_cmd_timeout",
                      std::bind(&TestScriptParse::test_parse_line_cmd_timeout,
                                this));
        register_test("parse_line_cmd_unknown",
                      std::bind(&TestScriptParse::test_parse_line_cmd_unknown,
                                this));
    }

    void test_next_line()
    {
        plux::ScriptParseCtx ctx;

        // ensure that last line (without newline) is parsed
        std::istringstream is1("last line");
        set_is(&is1);
        ASSERT_EQUAL("last line", true, next_line(ctx));
        ASSERT_EQUAL("last line", "last line", ctx.line);
        ASSERT_EQUAL("last line", false, next_line(ctx));

        // parse line with only - on it
        std::istringstream is2("     -\n     !test\n");
        set_is(&is2);
        ASSERT_EQUAL("only -", true, next_line(ctx));
        ASSERT_EQUAL("only -", "     -", ctx.line);
        ASSERT_EQUAL("only -", true, next_line(ctx));
        ASSERT_EQUAL("only -", "     !test", ctx.line);
        ASSERT_EQUAL("only -", false, next_line(ctx));

        // skip blank/empty lines and comments
        std::istringstream is3("\n     \n     #test\nlast");
        set_is(&is3);
        ASSERT_EQUAL("skip blank", true, next_line(ctx));
        ASSERT_EQUAL("skip blank", "last", ctx.line);
        ASSERT_EQUAL("skip blank", false, next_line(ctx));
    }

    void test_parse()
    {
        // FIXME: script does not start with [doc]
        // FIXME: not [enddoc] as first [ after [doc]
        // FIXME: unexpected content in headers
        // FIXME: unexpected content in shell
        // FIXME: unexpected content in cleanup

        // FIXME: verify shell is cleanup after [cleanup]
    }

    void test_parse_function()
    {
        std::istringstream is1("!echo true\n"
                               "?true\n"
                               "[endfunction]\n");
        set_is(&is1);
        auto line = parse_function(ctx("[function no-args]"));
        auto fun = dynamic_cast<plux::Function*>(line);
        ASSERT_EQUAL("no args", true, fun != nullptr);
        ASSERT_EQUAL("no args", "no-args", fun->name());
        ASSERT_EQUAL("no args", 0, fun->num_args());
        ASSERT_EQUAL("no args", 2, fun->line_end() - fun->line_begin());
        delete line;

        std::istringstream is2("!echo $arg1 $arg2\n"
                               "[endfunction]\n");
        set_is(&is2);
        line = parse_function(ctx("[function args arg1 arg2]"));
        fun = dynamic_cast<plux::Function*>(line);
        ASSERT_EQUAL("args", true, fun != nullptr);
        ASSERT_EQUAL("args", "args", fun->name());
        ASSERT_EQUAL("args", 2, fun->num_args());
        ASSERT_EQUAL("args", "arg1", *fun->args_begin());
        ASSERT_EQUAL("args", "arg2", *(fun->args_begin() + 1));
        ASSERT_EQUAL("args", 1, fun->line_end() - fun->line_begin());
        delete line;
    }

    void test_parse_config()
    {
        auto line = parse_config(ctx("[config require=V1]"));
        auto hdr = dynamic_cast<plux::HeaderConfigRequire*>(line);
        ASSERT_EQUAL("var only", true, hdr != nullptr);
        ASSERT_EQUAL("var only", "V1", hdr->key());
        ASSERT_EQUAL("var only", "", hdr->val());
        delete line;

        line = parse_config(ctx("[config require=V1=V2]"));
        hdr = dynamic_cast<plux::HeaderConfigRequire*>(line);
        ASSERT_EQUAL("var only", true, hdr != nullptr);
        ASSERT_EQUAL("var only", "V1", hdr->key());
        ASSERT_EQUAL("var only", "V2", hdr->val());
        delete line;
    }

    void test_parse_shell()
    {
        std::string name;
        ASSERT_EQUAL("valid", true,
                     parse_shell(ctx("[shell test]"), name));
        ASSERT_EQUAL("valid", std::string("test"), name);

        ASSERT_EQUAL("valid with offset", true,
                     parse_shell(ctx("[shell with-dash]"), name));
        ASSERT_EQUAL("valid with offset", "with-dash", name);

        try {
            parse_shell(ctx("[shell missing-end"), name);
            ASSERT_EQUAL("missing end", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("missing end",
                         "shell command does not end with ]", ex.error());
        }

        try {
            parse_shell(ctx("[shell invalid/char]"), name);
            ASSERT_EQUAL("invalid char", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("invalid char",
                         "invalid shell name: invalid/char. only "
                         "A-Z, a-z, 0-9, - and _ allowed", ex.error());
        }

        try {
            parse_shell(ctx("[shell cleanup]"), name);
            ASSERT_EQUAL("reserved name (cleanup)", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("reserved name (cleanup)",
                         "invalid shell name: cleanup. only "
                         "A-Z, a-z, 0-9, - and _ allowed", ex.error());
        }

        ASSERT_EQUAL("not shell", false,
                     parse_shell(ctx("[cleanup]"), name));
    }

    void test_parse_line_call()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("[call no-args]"));
        auto cline = dynamic_cast<plux::LineCall*>(line);
        ASSERT_EQUAL("call", true, cline != nullptr);
        ASSERT_EQUAL("call", "no-args", cline->name());
        ASSERT_EQUAL("call", 0, cline->num_args());
        delete line;

        line = parse_line_cmd(ctx("[call args one two]"));
        cline = dynamic_cast<plux::LineCall*>(line);
        ASSERT_EQUAL("call, args", true, cline != nullptr);
        ASSERT_EQUAL("call, args", "args", cline->name());
        ASSERT_EQUAL("call, args", 2, cline->num_args());
        delete line;
    }

    void test_parse_line_cmd_output()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("!echo 'test'"));
        auto oline = dynamic_cast<plux::LineOutput*>(line);
        ASSERT_EQUAL("output", true, oline != nullptr);
        ASSERT_EQUAL("output", std::string("echo 'test'\n"),
                     oline->output());
        delete line;
    }

    void test_parse_line_cmd_error()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("-"));
        auto eline = dynamic_cast<plux::LineSetErrorPattern*>(line);
        ASSERT_EQUAL("clear error", true, eline != nullptr);
        ASSERT_EQUAL("clear error", "", eline->pattern());
        delete line;

        line = parse_line_cmd(ctx("-error"));
        eline = dynamic_cast<plux::LineSetErrorPattern*>(line);
        ASSERT_EQUAL("set error", true, eline != nullptr);
        ASSERT_EQUAL("set error", "error", eline->pattern());
        delete line;
    }

    void test_parse_line_cmd_match()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("???exact match"));
        auto eline = dynamic_cast<plux::LineExactMatch*>(line);
        ASSERT_EQUAL("exact match", true, eline != nullptr);
        ASSERT_EQUAL("exact match", "exact match",
                     eline->pattern());
        delete line;

        line = parse_line_cmd(ctx("??match $HOME"));
        auto vline = dynamic_cast<plux::LineVarMatch*>(line);
        ASSERT_EQUAL("var match", true, vline != nullptr);
        ASSERT_EQUAL("var match", "match $HOME",
                     vline->pattern());
        delete line;

        line = parse_line_cmd(ctx("?^A.*$var$"));
        auto rline = dynamic_cast<plux::LineRegexMatch*>(line);
        ASSERT_EQUAL("regex match", true, rline != nullptr);
        ASSERT_EQUAL("regex match", "^A.*$var$",
                     rline->pattern());
        delete line;
    }

    void test_parse_line_cmd_global()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("[global my=value]"));
        auto gline = dynamic_cast<plux::LineVarAssignGlobal*>(line);
        ASSERT_EQUAL("global", true, gline != nullptr);
        ASSERT_EQUAL("global", "my", gline->key());
        ASSERT_EQUAL("global", "value", gline->val());
        delete line;
    }

    void test_parse_line_cmd_local()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("[local my=value]"));
        auto lline = dynamic_cast<plux::LineVarAssignShell*>(line);
        ASSERT_EQUAL("local", true, lline != nullptr);
        ASSERT_EQUAL("local", "my-shell", lline->shell());
        ASSERT_EQUAL("local", "my", lline->key());
        ASSERT_EQUAL("local", "value", lline->val());
        delete line;
    }

    void test_parse_line_cmd_timeout()
    {
        plux::Line* line;

        line = parse_line_cmd(ctx("[timeout]"));
        auto tline = dynamic_cast<plux::LineTimeout*>(line);
        ASSERT_EQUAL("timeout default", true,
                     tline != nullptr);
        ASSERT_EQUAL("timeout default",
                     plux::default_timeout_ms, tline->timeout());
        delete line;

        line = parse_line_cmd(ctx("[timeout 2]"));
        tline = dynamic_cast<plux::LineTimeout*>(line);
        ASSERT_EQUAL("timeout 2s", true,
                     tline != nullptr);
        ASSERT_EQUAL("timeout 2s", 2000,
                     tline->timeout());
        delete line;

        try {
            parse_line_cmd(ctx("[timeout nan]"));
            ASSERT_EQUAL("timeout nan", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("timeout nan",
                         "invalid timeout, not a valid number", ex.error());
        }
    }

    void test_parse_line_cmd_unknown()
    {
        try {
            parse_line_cmd(ctx("% unknown command"));
            ASSERT_EQUAL("unknown", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("unknown", "unexpected content",
                         ex.error());
        }

        try {
            parse_line_cmd(ctx("[unknown]"));
            ASSERT_EQUAL("unknown", false, true);
        } catch (plux::ScriptParseError& ex) {
            ASSERT_EQUAL("unknown", "unexpected content, unsupported function",
                         ex.error());
        }
    }

    plux::ScriptParseCtx& ctx(const std::string& line)
    {
        _ctx.shell = "my-shell";
        _ctx.start = 2;
        _ctx.line = "  " + line;
        return _ctx;
    }

private:
    plux::ScriptParseCtx _ctx;
};

int main(int argc, char* argv[])
{
    TestScriptParseCtx test_script_parse_ctx;
    std::istringstream is("");
    plux::ScriptEnv script_env;
    TestScriptParse test_script_parse(&is, script_env);

    try {
        return TestSuite::main(argc, argv);
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
