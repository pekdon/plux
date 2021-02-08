#ifndef _PLUX_HH_
#define _PLUX_HH_

#include <exception>
#include <map>
#include <string>

namespace plux {

    /**
     * Base class for all exceptions thrown by PLUX code.
     */
    class PluxException : public std::exception {
    public:
        PluxException() { }
        virtual ~PluxException() { }
        virtual std::string info() const = 0; // { return std::string(); }
        virtual std::string to_string() const {
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

    extern const unsigned int default_timeout_ms;
    extern const std::string empty_string;
    extern const std::map<std::string, std::string> default_env;

};

#endif // _PLUX_HH_
