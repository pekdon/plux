#include <iostream>

#include "test.hh"
#include "script.hh"
#include "script_header.hh"
#include "script_run.hh"
#include "shell_log.hh"

/**
 * ShellCtx user for testing.
 */
class ShellCtxTest : public plux::ShellCtx {
public:
    ShellCtxTest()
        : _name("my-shell"),
          _timeout_ms(-1),
          _is_alive(true),
          _exitstatus(-1)
    {
    }

    virtual ~ShellCtxTest() { }

    pid_t pid() const override { return 1; }
    bool is_alive() const override { return _is_alive; }
    void set_alive(bool alive, int exitstatus) override
    {
        _is_alive = alive;
        _exitstatus = exitstatus;
    }
    int exitstatus() const { return _exitstatus; }
    int fd_input() const override { return -1; }
    int fd_output() const override { return -1; }
    void stop() override { }

    virtual const std::string& name() const override { return _name; }
    virtual void progress_log(const std::string& msg) override { }
    virtual void progress_log(const std::string& context,
                              const std::string& msg) override { }

    unsigned int timeout() const override { return _timeout_ms; }
    virtual void set_timeout(unsigned int timeout_ms) override {
        _timeout_ms = timeout_ms;
    }

    virtual void set_error_pattern(const std::string& pattern) override {
        _error_pattern = pattern;
    }

    std::vector<std::string>& input() { return _input; }

    virtual bool input(const std::string& data) override {
        _input.push_back(data);
        return true;
    }
    virtual void output(const char* data, ssize_t size) override { }

    virtual line_it line_begin() override { return _lines.begin(); }
    virtual line_it line_end() override { return _lines.end(); }
    virtual void line_consume_until(line_it it) override { }

    virtual const std::string& buf() const override {
        return plux::empty_string;
    }
    virtual void consume_buf() override { }

private:
    std::string _name;
    unsigned int _timeout_ms;
    bool _is_alive;
    int _exitstatus;
    std::string _error_pattern;
    plux::ShellCtx::line_vector _lines;

    std::vector<std::string> _input;
};

class TestLine : public plux::Line,
                 public TestSuite {
public:
    TestLine()
        : Line(":memory:", 0, ""),
          TestSuite("Line")
    {
        register_test("expand_var",
                      std::bind(&TestLine::test_expand_var, this));
        register_test("append_var_val",
                      std::bind(&TestLine::test_append_var_val, this));
    }

    virtual ~TestLine() { }

    virtual plux::LineRes run(plux::ShellCtx& ctx,
                              plux::ShellEnv& env) override {
        return plux::LineRes(plux::RES_ERROR);
    }

    void test_expand_var()
    {
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);
        env.set_env("", "WHERE", plux::VAR_SCOPE_GLOBAL, "world");
        env.set_env("", "with space", plux::VAR_SCOPE_GLOBAL, "planet");
        env.set_env("", "glob1", plux::VAR_SCOPE_GLOBAL, "value");

        ASSERT_EQUAL("expand_var, no var", "test",
                     expand_var(env, "shell", "test"));
        ASSERT_EQUAL("expand_var, var", "hello world",
                     expand_var(env, "shell", "hello $WHERE"));
        ASSERT_EQUAL("expand_var, curly var", "hello planet",
                     expand_var(env, "shell", "hello ${with space}"));
        ASSERT_EQUAL("expand_var, ==", "==value==",
                     expand_var(env, "shell", "==$glob1=="));
        ASSERT_EQUAL("expand_var, normal then curly",
                     "hello worldplanet",
                     expand_var(env, "shell", "hello $WHERE${with space}"));

        try {
            expand_var(env, "shell", "invalid $ is empty");
            ASSERT_EQUAL("expand_var, empty", false, true);
        } catch (plux::ScriptError& ex) {
            ASSERT_EQUAL("expand_var, empty",
                         "empty variable name", ex.error());
        }

        try {
            expand_var(env, "shell", "my ${} var");
            ASSERT_EQUAL("expand_var, curly empty", false, true);
        } catch (plux::ScriptError& ex) {
            ASSERT_EQUAL("expand_var, curly empty",
                         "empty variable name", ex.error());
        }

        try {
            expand_var(env, "shell", "end ${end");
            ASSERT_EQUAL("expand_var, curly incomplete", false, true);
        } catch (plux::ScriptError& ex) {
            ASSERT_EQUAL("expand_var, curly incomplete",
                         "end of line while scanning for }", ex.error());
        }

        ASSERT_EQUAL("expand_var, skip end $", "^complete$",
                     expand_var(env, "shell", "^complete$"));

        ASSERT_EQUAL("expand_var, multi", "hello planet and world",
                     expand_var(env, "shell",
                                "hello ${with space} and $WHERE"));
    }

    void test_append_var_val()
    {
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);
        std::string val("with () and [] and . and * and ? and {} and |");
        env.set_env("", "VAR", plux::VAR_SCOPE_GLOBAL, val);

        std::string res;
        append_var_val(env, "shell-name", res, "VAR");
        ASSERT_EQUAL("verbatim value", val, res);
        std::string res_escaped;
        std::string val_escaped("with \\(\\) and \\[\\] and \\. and \\* and "
                                "\\? and \\{\\} and \\|");
        append_var_val(env, "shell-name", res_escaped, "=VAR");
        ASSERT_EQUAL("escaped value", val_escaped, res_escaped);
    }

    virtual std::string to_string() const override { return "TestLine"; }
};

class TestHeaderConfigRequire : public plux::HeaderConfigRequire,
                                public TestSuite
{
public:
    TestHeaderConfigRequire()
        : plux::HeaderConfigRequire(":memory:", 0, "key", ""),
          TestSuite("HeaderConfigRequire")
    {
        register_test("run",
                      std::bind(&TestHeaderConfigRequire::test_run, this));
    }
    virtual ~TestHeaderConfigRequire() { }

    void test_run()
    {
        ShellCtxTest ctx;
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);

        ASSERT_EQUAL("run", plux::RES_ERROR, run(ctx, env).status());
        env.set_env("", "key", plux::VAR_SCOPE_GLOBAL, "any");
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        env.set_env("", "key", plux::VAR_SCOPE_GLOBAL, "value");
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());

        set_val("value");
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        env.set_env("", "key", plux::VAR_SCOPE_GLOBAL, "any");
        ASSERT_EQUAL("run", plux::RES_ERROR, run(ctx, env).status());
    }
};

class TestScriptLine : public plux::Line,
                       public TestSuite
{
public:
    TestScriptLine()
        : plux::Line(":memory:", 0, ""),
          TestSuite("ScriptLine")
    {
    }
    virtual ~TestScriptLine() { }

    virtual std::string to_string() const override { return "TestScriptLine"; }
    virtual plux::LineRes run(plux::ShellCtx& ctx,
                              plux::ShellEnv& env) override {
        return plux::LineRes(plux::RES_ERROR);
    }
};

class TestLineVarAssignGlobal : public plux::LineVarAssignGlobal,
                                public TestSuite
{
public:
    TestLineVarAssignGlobal()
        : plux::LineVarAssignGlobal(":memory:", 0, "", "key", "global-val"),
          TestSuite("LineVarAssignGlobal")
    {
        register_test("run",
                      std::bind(&TestLineVarAssignGlobal::test_run, this));
    }
    virtual ~TestLineVarAssignGlobal() { }

    void test_run()
    {
        ShellCtxTest ctx;
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);

        std::string val;
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        ASSERT_EQUAL("run", true, env.get_env("", "key", val));
        ASSERT_EQUAL("run", "global-val", val);
    }
};

class TestLineVarAssignShell : public plux::LineVarAssignShell,
                               public TestSuite
{
public:
    TestLineVarAssignShell()
        : plux::LineVarAssignShell(":memory:", 0, "shell", "key", "local-val"),
          TestSuite("LineVarAssignShell")
    {
        register_test("run",
                      std::bind(&TestLineVarAssignShell::test_run, this));
    }

    ~TestLineVarAssignShell() { }

    void test_run()
    {
        ShellCtxTest ctx;
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);

        std::string val;
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        ASSERT_EQUAL("run", true, env.get_env("my-shell", "key", val));
        ASSERT_EQUAL("run", "local-val", val);
        ASSERT_EQUAL("run", false, env.get_env("my-shell-2", "key", val));
    }
};

class TestLineOutput : public plux::LineOutput,
                       public TestSuite {
public:
    TestLineOutput()
        : plux::LineOutput(":memory:", 0, "shell", "not-used"),
          TestSuite("LineOutput")
    {
        register_test("run", std::bind(&TestLineOutput::test_run, this));
    }

    virtual ~TestLineOutput() { }

    void test_run()
    {
        ShellCtxTest ctx;
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);
        env.set_env("", "glob1", plux::VAR_SCOPE_GLOBAL, "value");

        set_output("plain\n");
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        ASSERT_EQUAL("run", 1, ctx.input().size());
        ASSERT_EQUAL("run", "plain\n", ctx.input()[0]);

        set_output("echo ==$glob1==\n");
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        ASSERT_EQUAL("run", 2, ctx.input().size());
        ASSERT_EQUAL("run", "echo ==value==\n", ctx.input()[1]);
    }
};

class TestLineVarMatch : public plux::LineVarMatch,
                         public TestSuite {
public:
    TestLineVarMatch()
        : plux::LineVarMatch(":memory:", 0, "shell", "not-used"),
          TestSuite("LineVarMatch")
    {
        register_test("match", std::bind(&TestLineVarMatch::test_match, this));
    }

    virtual ~TestLineVarMatch() { }

    void test_match()
    {
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);
        env.set_env("", "VALUE", plux::VAR_SCOPE_GLOBAL, "world");

        set_pattern("SH-PROMPT:");
        ASSERT_EQUAL("match", true,
                     match(env, "shell", "SH-PROMPT:", true));

        set_pattern("hello $VALUE");
        ASSERT_EQUAL("match var", true,
                     match(env, "shell", "hello world", true));
        ASSERT_EQUAL("match var", false,
                     match(env, "shell", "hello planet", true));
    }
};

class TestLineRegexMatch : public plux::LineRegexMatch,
                           public TestSuite {
public:
    TestLineRegexMatch()
        : plux::LineRegexMatch(":memory:", 0, "shell", "not-used"),
          TestSuite("LineRegexMatch")
    {
        register_test("match",
                      std::bind(&TestLineRegexMatch::test_match, this));
    }
    virtual ~TestLineRegexMatch() { }

    void test_match()
    {
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);

        set_pattern("SH-PROMPT:");
        ASSERT_EQUAL("match full", true,
                     match(env, "shell", "SH-PROMPT:", true));
        ASSERT_EQUAL("match full", true,
                     match(env, "shell", "SH-PROMPT:", false));

        set_pattern("partial");
        ASSERT_EQUAL("match partial", true,
                     match(env, "shell", "1partial2", true));
        ASSERT_EQUAL("match partial", true,
                     match(env, "shell", "1partial2", false));

        set_pattern("^complete$");
        ASSERT_EQUAL("match anchor", true,
                     match(env, "shell", "complete", true));
        ASSERT_EQUAL("match anchor", false,
                     match(env, "shell", "complete", false));
        ASSERT_EQUAL("match anchor", false,
                     match(env, "shell", "complete and more", true));
        ASSERT_EQUAL("match anchor", false,
                     match(env, "shell", "more and complete", true));

        set_pattern("[0-K");
        try {
            match(env, "shell", "some input", true);
            ASSERT_EQUAL("invalid", false, true);
        } catch (plux::ScriptError& ex) {
            ASSERT_EQUAL("invalid", "regex failed: ",
                         ex.error().substr(0, 14));
        }

        set_pattern("hello ([a-z]+) and ([0-9]+)!");
        ASSERT_EQUAL("extract group", true,
                     match(env, "shell", "hello world and 2021!", true));
        std::string val;
        ASSERT_EQUAL("extract group", true, env.get_env("shell", "1", val));
        ASSERT_EQUAL("extract group", "world", val);
        ASSERT_EQUAL("extract group", true, env.get_env("shell", "2", val));
        ASSERT_EQUAL("extract group", "2021", val);
    }
};

class TestLineTimeout : public plux::LineTimeout,
                        public TestSuite {
public:
    TestLineTimeout()
        : plux::LineTimeout(":memory", 0, "shell", 1000),
          TestSuite("LineTimeout")
    {
        register_test("run", std::bind(&TestLineTimeout::test_run, this));
    }

    virtual ~TestLineTimeout() { }

    void test_run()
    {
        ShellCtxTest ctx;
        plux::env_map os_env;
        plux::ShellEnvImpl env(os_env);
        ASSERT_EQUAL("run", plux::RES_OK, run(ctx, env).status());
        ASSERT_EQUAL("run", _timeout_ms, ctx.timeout());
    }
};

int main(int argc, char *argv[])
{
    TestLine test_line;
    TestHeaderConfigRequire test_header_config_require;
    TestScriptLine test_script_line;
    TestLineVarAssignGlobal test_assign_global;
    TestLineVarAssignShell test_assign_shell;
    TestLineOutput test_output;
    TestLineVarMatch test_var_match;
    TestLineRegexMatch test_re_match;
    TestLineTimeout test_timeout;

    try {
        return TestSuite::main(argc, argv);
    } catch (plux::PluxException &ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
