#pragma once

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

    std::string path_basename(std::string path);
    std::string path_dirname(std::string path);
    std::string path_join(const std::string& p1, const std::string& p2);

    extern const unsigned int default_timeout_ms;
    extern const std::string empty_string;
    extern const std::map<std::string, std::string> default_env;
}
