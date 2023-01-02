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
    }

    void test_str_split()
    {
        std::vector<std::string> toks;
        ASSERT_EQUAL("num elements",
                     3, plux::str_split("one two three", toks));
        ASSERT_EQUAL("mismatch", "one", toks[0]);
        ASSERT_EQUAL("mismatch", "two", toks[1]);
        ASSERT_EQUAL("mismatch", "three", toks[2]);
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
