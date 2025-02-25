#pragma once

#include <cstring>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace plux
{
    class sview {
    public:
        sview(const char *data, size_t len)
            : _data(data),
              _len(len)
        {
        }

        const char* data() const { return _data; }
        const size_t len() const { return _len; }

        bool operator==(const std::string &str) const
        {
            return str.size() == _len && memcmp(_data, str.c_str(), _len) == 0;
        }

    private:
        const char *_data;
        size_t _len;
    };

    size_t str_split(const std::string& str, size_t pos,
                     std::vector<std::string>& toks);
    std::string str_unescape(const std::string& src, size_t pos, size_t len);
    size_t str_scan(const std::string& str, size_t pos,
                    const std::string& end);
    sview str_view(const std::string& str, size_t pos, size_t len);
}

inline bool operator==(const char *lhs, const plux::sview& rhs)
{
    return rhs == lhs;
}

inline bool operator!=(const char *lhs, const plux::sview& rhs)
{
    return ! (rhs == lhs);
}

inline std::ostream& operator<<(std::ostream& ostream,
                                const plux::sview& sview)
{
    ostream.write(sview.data(), sview.len());
    return ostream;
}
