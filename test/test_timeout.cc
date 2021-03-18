#include <iostream>

extern "C" {
#include <unistd.h>
}

#include "test.hh"
#include "timeout.hh"
#include "plux.hh"

class TestTimeout : public TestSuite {
public:
    TestTimeout()
        : TestSuite("timeout")
    {
        register_test("restart", std::bind(&TestTimeout::test_restart, this));
    }
    
    void test_restart()
    {
        auto t = plux::Timeout(1000);
        ASSERT_EQUAL("ms > 0", true, t.get_ms_until_timeout() > 0);
        sleep(1);
        ASSERT_EQUAL("ms == 0", true, t.get_ms_until_timeout() <= 0);
        t.restart();
        ASSERT_EQUAL("ms > 0", true, t.get_ms_until_timeout() > 0);
    }
};

int main(int argc, char* argv[])
{
    TestTimeout test_timeout;
    try {
        return TestSuite::main(argc, argv);
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
