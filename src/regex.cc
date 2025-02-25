#include "regex.hh"

namespace plux
{
#ifdef WORKING_CXX_REGEX

    bool regex_search(const std::string& s, const regex& e)
    {
        return std::regex_search(s, e);
    }

    bool regex_search(const std::string& s, smatch& matches, const regex& e)
    {
        return std::regex_search(s, matches, e);
    }

    bool regex_match(const std::string& s, const regex& e)
    {
        return std::regex_match(s, e);
    }

#else // ! WORKING_CXX_REGEX

    regex::regex(void)
        : _compiled(false)
    {
    }

    regex::regex(const std::string& pattern)
        : _compiled(false)
    {
        set(pattern);
    }

    regex::~regex(void)
    {
        free();
    }

    void regex::set(const std::string& pattern)
    {
        free();

        if (pattern.empty()) {
            // do nothing, empty pattern ok but will not match
            return;
        }

        std::string ext_pattern = transform(pattern);

        int flags = REG_EXTENDED;
        int err = regcomp(&_re, ext_pattern.c_str(), flags);
        if (! err) {
            _compiled = true;
        } else {
            char buf[128] = {0};
            regerror(err, &_re, buf, sizeof(buf));
            throw regex_error(buf);
        }
    }

    std::string regex::transform(const std::string& pattern)
    {
        std::string ext_pattern;
        bool in_escape = false;
        int in_group = 0;
        for (size_t i = 0; i < pattern.size(); i++) {
            if (in_escape) {
                // \d works
                switch (pattern[i]) {
                case 'D':
                    transform_add_group(ext_pattern, in_group, "^[:digit:]");
                    break;
                case 's':
                    transform_add_group(ext_pattern, in_group, " \n\r\t");
                    break;
                case 'S':
                    transform_add_group(ext_pattern, in_group, "^ \n\r\t");
                    break;
                case 'w':
                    transform_add_group(ext_pattern, in_group, "[:alnum:]_");
                    break;
                case 'W':
                    transform_add_group(ext_pattern, in_group, "^[:alnum:]_");
                    break;
                default:
                    ext_pattern += '\\';
                    ext_pattern += pattern[i];
                    break;
                }
                in_escape = false;
            } else if (pattern[i] == '\\') {
                in_escape = true;
            } else {
                if (pattern[i] == '[') {
                    in_group++;
                } else if (pattern[i] == ']') {
                    in_group--;
                }
                ext_pattern += pattern[i];
            }
        }
        return ext_pattern;
    }

    void regex::transform_add_group(std::string &pattern, int in_group,
                                    const char* group)
    {
        if (in_group > 0) {
            pattern += group;
        } else {
            pattern += '[';
            pattern += group;
            pattern += ']';
        }
    }

    void regex::free(void)
    {
        if (_compiled) {
            regfree(&_re);
            _compiled = false;
        }
    }

    bool regex_search(const std::string& s, const regex& e)
    {
        if (! e.compiled()) {
            return false;
        }

        return regexec(e.re(), s.c_str(), 0, nullptr, 0) == 0;
    }

    bool regex_search(const std::string& s, smatch& matches, const regex& e)
    {
        if (! e.compiled()) {
            return false;
        }

        const size_t max_ref = e.nsub() + 1;
        auto r_matches = new regmatch_t[max_ref];
        if (regexec(e.re(), s.c_str(), max_ref, r_matches, 0)) {
            delete [] r_matches;
            return false;
        }

        for (size_t i = 0; i < max_ref; i++) {
            std::string match;
            if (r_matches[i].rm_so == -1 || r_matches[i].rm_eo == -1) {
                // no match, leave as empty string
            } else {
                auto size = r_matches[i].rm_eo - r_matches[i].rm_so;
                match = std::string(s.c_str() + r_matches[i].rm_so, size);
            }
            matches.push_back(sub_match(match));
        }

        delete [] r_matches;

        return true;
    }

    bool regex_match(const std::string& s, const regex& e)
    {
        regmatch_t r_matches[1];
        if (! e.compiled()
            || regexec(e.re(), s.c_str(), 1, r_matches, 0)) {
            return false;
        }

        return (r_matches[0].rm_so == 0)
            && (static_cast<size_t>(r_matches[0].rm_eo) == s.size());
    }

#endif // WORKING_CXX_REGEX
}
