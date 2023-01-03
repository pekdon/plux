#include "plux.hh"

extern "C" {
#include <libgen.h>
#include <time.h>
}

namespace plux
{
    PluxException::PluxException(void) throw()
    {
    }

    PluxException::~PluxException(void) throw()
    {
    }

    /**
     * Format current time as string in format: yyyy-mm-dd HH:MM:SS
     */
    std::string format_timestamp(void)
    {
        time_t now = time(nullptr);
        struct tm tm;
        gmtime_r(&now, &tm);

        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buf);
    }

    const unsigned int default_timeout_ms = 60000;
    const std::string empty_string;

    const std::map<std::string, std::string> default_env = {
        {"_TAB_", "\t"},
        {"_CTRL_C_", "\003"}
    };
}
