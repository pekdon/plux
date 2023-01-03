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

    /**
     * Get basename from path.
     */
    std::string path_basename(const std::string& path)
    {
        std::string path_copy(path);
        return basename(const_cast<char*>(path_copy.c_str()));
    }

    /**
     * Get dirname from path.
     */
    std::string path_dirname(const std::string& path)
    {
        std::string path_copy(path);
        return dirname(const_cast<char*>(path_copy.c_str()));
    }

    /**
     * Join two path elements.
     */
    std::string path_join(const std::string& p1, const std::string& p2)
    {
        if (p1.size() == 0) {
            return p2;
        } else if (p2.size() == 0) {
            return p1;
        } else if (p1[p1.size() - 1] == '/' && p2[0] == '/') {
            return p1 + p2.substr(1);
        } else if(p1[p1.size() - 1] == '/' || p2[0] == '/') {
            return p1 + p2;
        } else {
            return p1 + "/" + p2;
        }
    }

    const unsigned int default_timeout_ms = 60000;
    const std::string empty_string;

    const std::map<std::string, std::string> default_env = {
        {"_TAB_", "\t"},
        {"_CTRL_C_", "\003"}
    };
}
