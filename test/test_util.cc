#include "test.hh"
#include "plux.hh"
#include "util.hh"

class TestUtil : public TestSuite {
public:
    TestUtil()
        : TestSuite("Util")
    {
        register_test("Defer",
                      std::bind(&TestUtil::test_Defer, this));
    }

    void test_Defer()
    {
        int count = 0;
        std::function<void()> fun = [&count] { count++; };
        {
            Defer defer(fun);
        }
        ASSERT_EQUAL("defer", 1, count);

        {
            Defer defer(fun);
            defer.cancel();
        }
        ASSERT_EQUAL("defer cancelled", 1, count);
    }
};

int main(int argc, char* argv[])
{
    try {
        TestUtil test_util;
        TestSuite::main(argc, argv);
        return 0;
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
