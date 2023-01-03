#include "test.hh"
#include "plux.hh"

class TestPlux : public TestSuite {
public:
    TestPlux()
        : TestSuite("Plux")
    {
        register_test("path_join", std::bind(&TestPlux::test_path_join, this));
    }

    virtual ~TestPlux() { }

    void test_path_join()
    {
        ASSERT_EQUAL("both empty", "", plux::path_join("", ""));
        ASSERT_EQUAL("p1 empty", "p2", plux::path_join("", "p2"));
        ASSERT_EQUAL("p2 empty", "p1", plux::path_join("p1", ""));
        ASSERT_EQUAL("both slash", "/p2", plux::path_join("/p1/", "/p2"));
        ASSERT_EQUAL("p1 slash", "/p1/p2", plux::path_join("/p1/", "p2"));
        ASSERT_EQUAL("p2 slash", "/p2", plux::path_join("/p1", "/p2"));
        ASSERT_EQUAL("no slash", "/p1/p2", plux::path_join("/p1", "p2"));
    }
};

int main(int argc, char *argv[])
{
    TestPlux test_plux;
    try {
        return TestSuite::main(argc, argv);
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
