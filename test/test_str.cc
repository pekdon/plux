#include "test.hh"
#include "plux.hh"
#include "str.hh"

class TestStr : public TestSuite {
public:
    TestStr()
        : TestSuite("Str")
    {
        register_test("str_split",
                      std::bind(&TestStr::test_str_split, this));
        register_test("str_scan",
                      std::bind(&TestStr::test_str_scan, this));
        register_test("str_view",
                      std::bind(&TestStr::test_str_view, this));
    }

    void test_str_split()
    {
        std::vector<std::string> toks;
        ASSERT_EQUAL("num elements",
                     3, plux::str_split("one two three", 0, toks));
        ASSERT_EQUAL("mismatch", "one", toks[0]);
        ASSERT_EQUAL("mismatch", "two", toks[1]);
        ASSERT_EQUAL("mismatch", "three", toks[2]);
    }

    void test_str_scan()
    {
        size_t pos;
        pos = plux::str_scan("single space", 0, " ");
        ASSERT_EQUAL("single space", 6, pos);
        pos = plux::str_scan("hello \"my -- world\" -- end", 0, " -- ");
        ASSERT_EQUAL("quoted", 19, pos);
    }

    void test_str_view()
    {
        ASSERT_EQUAL("in range",
                     "my", plux::str_view("hello my world", 6, 2));
        ASSERT_EQUAL("past end", "orld", plux::str_view("world", 1, 10));
        ASSERT_EQUAL("out of range", "", plux::str_view("my", 10, 2));
    }
};

int main(int argc, char* argv[])
{
    try {
        TestStr test_str;
        TestSuite::main(argc, argv);
        return 0;
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
