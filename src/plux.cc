#include "plux.hh"

#include <sstream>

extern "C" {
#include <libgen.h>
#include <time.h>
}

static const uint64_t NSEC_PER_MSEC = 1000000;
static const uint64_t NSEC_PER_SEC = 1000000000;

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
     * Format difference between start and end as elapsed time in
     * human readable time.
     */
    std::string format_elapsed(const struct timespec &start,
                               const struct timespec &end)
    {
        std::stringstream buf;
        time_t sec = end.tv_sec - start.tv_sec;
        uint64_t nsec;
        if (start.tv_nsec > end.tv_nsec) {
            sec -= 1;
            nsec = end.tv_nsec + (NSEC_PER_SEC - start.tv_nsec);
        } else {
            nsec = end.tv_nsec - start.tv_nsec;
        }
        uint64_t msec = nsec / NSEC_PER_MSEC;

        if (sec > 3600) {
            buf << (sec / 3600) << "h";
            sec = sec % 3600;
        }
        if (sec > 60) {
            buf << (sec / 60) << "m";
            sec = sec % 60;
        }

        buf << sec << "s";
        if (msec > 0) {
            buf << msec << "ms";
        }
        return buf.str();
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
        } else if (p2[0] == '/') {
            return p2;
        } else if(p1[p1.size() - 1] == '/') {
            return p1 + p2;
        } else {
            return p1 + "/" + p2;
        }
    }

    const unsigned int default_timeout_ms = 60000;
    const std::string empty_string;

    const std::map<std::string, std::string> default_env = {
        {"_TAB_", "\t"},
        {"_CTRL_C_", "\003"},
        {"_COLOR_BOLD_", COLOR_BOLD},
        {"_COLOR_BLACK_", COLOR_BLACK},
        {"_COLOR_RED_", COLOR_RED},
        {"_COLOR_GREEN_", COLOR_GREEN},
        {"_COLOR_YELLOW_", COLOR_YELLOW},
        {"_COLOR_BLUE_", COLOR_BLUE},
        {"_COLOR_MAGENTA_", COLOR_MAGENTA},
        {"_COLOR_CYAN_", COLOR_CYAN},
        {"_COLOR_WHITE_", COLOR_WHITE},
        {"_COLOR_RESET_", COLOR_RESET}
    };
}
