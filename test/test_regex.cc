#include "test.hh"
#include "plux.hh"
#include "regex.hh"

class TestRegex : public TestSuite {
public:
    TestRegex()
        : TestSuite("Regex")
    {
        register_test("match",
                      std::bind(&TestRegex::test_match, this));
        register_test("search",
                      std::bind(&TestRegex::test_search, this));
        register_test("search_splus",
                      std::bind(&TestRegex::test_search_splus, this));
        register_test("search_dD",
                      std::bind(&TestRegex::test_search_dD, this));
        register_test("search_wW",
                      std::bind(&TestRegex::test_search_wW, this));
    }

    void test_match()
    {
        plux::regex re("hello");
        ASSERT_EQUAL("match", false, regex_match("hello world", re));
        ASSERT_EQUAL("match", true, regex_match("hello", re));
    }

    void test_search()
    {
        plux::regex re("^[\\s>]*==(.+)==$");

        const std::string line("> > > > ==do-start-xvfb==");
        ASSERT_EQUAL("search", true, regex_search(line, re));
        plux::smatch m;
        ASSERT_EQUAL("search (matches)", true, regex_search(line, m, re));
        ASSERT_EQUAL("match count", 2, m.size());
        ASSERT_EQUAL("match[0]", line, m[0]);
        ASSERT_EQUAL("match[1]", "do-start-xvfb", m[1]);
    }

    void test_search_splus()
    {
        plux::regex re("XTerm\\*background:\\s+#ffffff");

        const std::string line("XTerm*background:        #ffffff");
        ASSERT_EQUAL("search", true, regex_search(line, re));
    }

    void test_search_dD()
    {
        const std::string line("abc 123");

        plux::regex red("(\\d+)");
        plux::smatch md;
        ASSERT_EQUAL("d+", true, regex_search(line, md, red));
        ASSERT_EQUAL("d+", 2, md.size());
        ASSERT_EQUAL("d+", "123", md[1]);

        plux::regex reD("(\\D+)");
        plux::smatch mD;
        ASSERT_EQUAL("D+", true, regex_search(line, mD, reD));
        ASSERT_EQUAL("D+", 2, mD.size());
        ASSERT_EQUAL("D+", "abc ", mD[1]);
    }

    void test_search_wW()
    {
        const std::string line("!!!123_abc ");

        plux::regex rew("(\\w+)");
        plux::smatch mw;
        ASSERT_EQUAL("w+", true, regex_search(line, mw, rew));
        ASSERT_EQUAL("w+", 2, mw.size());
        ASSERT_EQUAL("w+", "123_abc", mw[1]);

        plux::regex reW("(\\W+)");
        plux::smatch mW;
        ASSERT_EQUAL("W+", true, regex_search(line, mW, reW));
        ASSERT_EQUAL("W+", 2, mW.size());
        ASSERT_EQUAL("W+", "!!!", mW[1]);
    }
};

int main(int argc, char* argv[])
{
    try {
        TestRegex test_str;
        TestSuite::main(argc, argv);
        return 0;
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
