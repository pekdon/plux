#include "test.hh"
#include "script_run.hh"

class TestShellEnvImpl : public plux::ShellEnvImpl,
                         public TestSuite {
public:
    explicit TestShellEnvImpl(const plux::env_map& env)
        :  plux::ShellEnvImpl(env),
           TestSuite("ShellEnvImpl")
    {
        register_test("get_env",
                      std::bind(&TestShellEnvImpl::test_get_env, this));
    }

    void test_get_env()
    {
        std::string val;

        ASSERT_EQUAL("unset", false, get_env("", "missing", val));

        ASSERT_EQUAL("os", true, get_env("", "key", val));
        ASSERT_EQUAL("os", "os-val", val);

        set_env("", "key", plux::VAR_SCOPE_GLOBAL, "global-val");
        ASSERT_EQUAL("global", true, get_env("", "key", val));
        ASSERT_EQUAL("global", "global-val", val);

        set_env("sh1", "key", plux::VAR_SCOPE_SHELL, "sh1-val");
        set_env("sh2", "key", plux::VAR_SCOPE_SHELL, "sh2-val");
        ASSERT_EQUAL("shell", true, get_env("sh1", "key", val));
        ASSERT_EQUAL("shell", "sh1-val", val);
        ASSERT_EQUAL("shell, fallback", true, get_env("sh3", "key", val));
        ASSERT_EQUAL("shell, fallback", "global-val", val);

        push_function();
        set_env("sh1", "key", plux::VAR_SCOPE_FUNCTION, "sh1-fun-val");
        set_env("sh2", "key", plux::VAR_SCOPE_FUNCTION, "sh2-fun-val");
        ASSERT_EQUAL("function", true, get_env("sh1", "key", val));
        ASSERT_EQUAL("function", "sh1-fun-val", val);
        ASSERT_EQUAL("function, fallback", true, get_env("sh3", "key", val));
        ASSERT_EQUAL("function, fallback", "global-val", val);

        pop_function();
        ASSERT_EQUAL("pop function", true, get_env("sh1", "key", val));
        ASSERT_EQUAL("pop function", "sh1-val", val);
    }
};

int main(int argc, char* argv[])
{
    try {
        plux::env_map env = {{"key", "os-val"}};
        TestShellEnvImpl test_shell_env_impl(env);
        TestSuite::main(argc, argv);
        return 0;
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
