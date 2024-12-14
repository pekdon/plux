#pragma once

#include <ctime>
#include <exception>
#include <map>
#include <string>

namespace plux
{
    /**
     * Base class for all exceptions thrown by PLUX code.
     */
    class PluxException {
    public:
        PluxException(void) throw();
        virtual ~PluxException(void) throw();

        virtual std::string info(void) const = 0;
        virtual std::string to_string(void) const {
            return "PluxException";
        }
    };

    /**
     * Character comparision.
     */
    struct nocase_compare {
        bool operator()(unsigned char c1, unsigned char c2) const {
            return tolower(c1) < tolower(c2);
        }
    };

    /**
     * Case insensitive less
     */
    struct case_insensitive_less {
        bool operator()(const std::string& lhs,
                        const std::string& rhs) const {
            return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                                rhs.begin(), rhs.end(),
                                                nocase_compare());
        }
    };

    std::string format_timestamp(void);
    std::string format_elapsed(const struct timespec &start,
                               const struct timespec &end);

    std::string path_basename(const std::string& path);
    std::string path_dirname(const std::string& path);
    std::string path_join(const std::string& p1, const std::string& p2);

    constexpr const char *COLOR_BOLD = "\033[1m";
    constexpr const char *COLOR_BLACK = "\033[30m";
    constexpr const char *COLOR_RED = "\033[31m";
    constexpr const char *COLOR_GREEN = "\033[32m";
    constexpr const char *COLOR_YELLOW = "\033[33m";
    constexpr const char *COLOR_BLUE = "\033[34m";
    constexpr const char *COLOR_MAGENTA = "\033[35m";
    constexpr const char *COLOR_CYAN = "\033[36m";
    constexpr const char *COLOR_WHITE = "\033[37m";
    constexpr const char *COLOR_RESET = "\033[0m";

    extern const unsigned int default_timeout_ms;
    extern const std::string empty_string;
    extern const std::map<std::string, std::string> default_env;
}
