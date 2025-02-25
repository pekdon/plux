#pragma once

#include "config.h"

#include <string>

#ifdef WORKING_CXX_REGEX
#include <regex>
#else // ! WORKING_CXX_REGEX
#include <vector>
#include <stdexcept>
extern "C" {
#include <regex.h>
}
#endif // WORKING_CXX_REGEX

namespace plux
{
#ifdef WORKING_CXX_REGEX

    typedef std::regex regex;
    typedef std::regex_error regex_error;
    typedef std::smatch smatch;

#else // ! WORKING_CXX_REGEX

    /**
     * C++ const_reference limited look-alike.
     */
    class sub_match {
    public:
        explicit sub_match(const std::string& str)
            : _str(str)
        {
        }

        const std::string& str(void) const { return _str; }

    private:
        std::string _str;
    };

    /**
     * C++11 smatch limited look-alike.
     */
    class smatch : public std::vector<sub_match> {
    public:
        virtual ~smatch(void) { }
    };

    /**
     * C++11 regex_error limited look-alike.
     */
    class regex_error : public std::runtime_error {
    public:
        explicit regex_error(const std::string& what)
            : std::runtime_error(what)
        {
        }
        virtual ~regex_error(void) { }
    };

    /**
     * C++11 regex limited look-alike using regcomp/regex from libc.
     */
    class regex {
    public:
        regex(void);
        explicit regex(const std::string& pattern);
        regex(const regex& regex) = delete;
        ~regex(void);

        bool compiled() const { return _compiled; }
        const regex_t* re(void) const { return &_re; }
        size_t nsub(void) const { return _re.re_nsub; }

        regex &operator=(const std::string& pattern) {
            set(pattern);
            return *this;
        }

    private:
        void set(const std::string& pattern);
        std::string transform(const std::string& pattern);
        void transform_add_group(std::string& pattern, int in_group,
                                 const char* group);
        void free();

        regex_t _re;
        bool _compiled;
    };

#endif // WORKING_CXX_REGEX

    bool regex_search(const std::string& s, const regex& e);
    bool regex_search(const std::string& s, smatch& matches, const regex& e);
    bool regex_match(const std::string& s, const regex& e);
}

#ifndef WORKING_CXX_REGEX
inline std::ostream& operator<<(std::ostream& ost, const plux::sub_match& sm)
{
    ost << sm.str();
    return ost;
}

inline bool operator==(const std::string& lhs, const plux::sub_match& rhs)
{
    return lhs == rhs.str();
}

inline bool operator==(const char* lhs, const plux::sub_match& rhs)
{
    return rhs.str() == lhs;
}
#endif // WORKING_CXX_REGEX
