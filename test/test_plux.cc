#include "test.hh"
#include "plux.hh"

class TestPlux : public TestSuite {
public:
    TestPlux()
        : TestSuite("Plux")
    {
        register_test("format_elapsed",
                      std::bind(&TestPlux::test_format_elapsed, this));
        register_test("path_join", std::bind(&TestPlux::test_path_join, this));
    }

    virtual ~TestPlux() { }

    void test_format_elapsed()
    {
        struct timespec start, end;
        start.tv_sec = 0;
        start.tv_nsec = 0;

        ASSERT_EQUAL("no time", "0s", plux::format_elapsed(start, start));

        end.tv_sec = 23;
        end.tv_nsec = 0;
        ASSERT_EQUAL("s", "23s", plux::format_elapsed(start, end));

        end.tv_sec = 723;
        end.tv_nsec = 0;
        ASSERT_EQUAL("m, s", "12m3s", plux::format_elapsed(start, end));

        end.tv_sec = 3723;
        end.tv_nsec = 0;
        ASSERT_EQUAL("h, m, s", "1h2m3s", plux::format_elapsed(start, end));

        end.tv_nsec = 500000000;
        ASSERT_EQUAL("h, m, s, ms", "1h2m3s500ms",
                     plux::format_elapsed(start, end));

        start.tv_nsec = 750000000;
        ASSERT_EQUAL("h, m, s, ms", "1h2m2s750ms",
                     plux::format_elapsed(start, end));
    }

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
