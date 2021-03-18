#include <iostream>
#include <vector>

#include "test.hh"
#include "log.hh"
#include "plux.hh"

class TestLog : public plux::Log, public TestSuite {
public:
    TestLog()
        : plux::Log(plux::LOG_LEVEL_INFO),
          TestSuite("Log")
    {
        register_test("operator", std::bind(&TestLog::test_operator, this));
    }

    virtual ~TestLog() { }

    void test_operator()
    {
        _lines.clear();
        *this << "Test" << "message1" << plux::LOG_LEVEL_INFO;
        ASSERT_EQUAL("plain", 1, _lines.size());
        ASSERT_EQUAL("plain", "INFO    Test: message1", _lines[0].substr(20));

        *this << "Test" << "message2" << plux::LOG_LEVEL_INFO;
        ASSERT_EQUAL("plain (clear buf)", 2, _lines.size());
        ASSERT_EQUAL("plain (clear buf)", "INFO    Test: message2", _lines[1].substr(20));

        _lines.clear();
        *this << "Test" << "message" << plux::LOG_LEVEL_DEBUG;
        ASSERT_EQUAL("under level", 0, _lines.size());
    }

protected:
    virtual void write(enum plux::log_level level, const std::string& full_msg)
    {
        _lines.push_back(full_msg);
    }

private:
    std::vector<std::string> _lines;
};

int main(int argc, char *argv[])
{
    TestLog test_log;
    try {
        return TestSuite::main(argc, argv);
    } catch (plux::PluxException& ex) {
        std::cerr << ex.to_string() << std::endl;
        return 1;
    }
}
