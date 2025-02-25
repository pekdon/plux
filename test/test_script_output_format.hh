#include "output_format.hh"

class TestOutputFormat : public TestSuite {
public:
    TestOutputFormat()
        : TestSuite("OutputFormat")
    {
        register_test("no_format",
                      std::bind(&TestOutputFormat::test_no_format, this));
        register_test("missing_arg",
                      std::bind(&TestOutputFormat::test_missing_arg, this));
        register_test("int_types",
                      std::bind(&TestOutputFormat::test_int_types, this));
        register_test("string",
                      std::bind(&TestOutputFormat::test_string, this));
        register_test("len",
                      std::bind(&TestOutputFormat::test_len, this));
    }

    virtual ~TestOutputFormat() { }

    void test_no_format()
    {
	    plux::OutputFormat of("verbatim string", {});
	    std::string res;
	    ASSERT_TRUE("no format", of.format(res));
	    ASSERT_EQUAL("no format", "verbatim string", res);
    }

    void test_missing_arg()
    {
	    std::string res;
	    plux::OutputFormat of("missing %s[0]", {});
	    ASSERT_FALSE("missing arg", of.format(res));
	    ASSERT_EQUAL("missing arg", "argument 0 missing", of.error());
    }

    void test_int_types()
    {
	    plux::OutputFormat of("i8 %i8[0]", {"65"});
	    std::string res;
	    ASSERT_TRUE("i8", of.format(res));
	    ASSERT_EQUAL("i8", "i8 A", res);
    }

    void test_string()
    {
	    plux::OutputFormat of("str %s[0] %s[1]", {"hello", "world"});
	    std::string res;
	    ASSERT_TRUE("str", of.format(res));
	    ASSERT_EQUAL("str", "str hello world", res);
    }

    void test_len()
    {
	    plux::OutputFormat of("%i8[len(0)]", {"my string1"});
	    std::string res;
	    ASSERT_TRUE("len", of.format(res));
	    ASSERT_EQUAL("len", "\n", res);
    }
};
