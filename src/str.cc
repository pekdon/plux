#include "str.hh"

/**
 * Split string into tokens supporting quotation.
 */
size_t plux::str_split(const std::string& str, size_t pos,
                       std::vector<std::string>& toks)
{
    bool in_tok = false;
    bool in_escape = false;
    char in_quote = 0;
    size_t start_size = toks.size();

    std::string tok;
    for (; pos < str.size(); ++pos) {
        char chr = str[pos];
        if (! in_tok && ! isspace(chr)) {
            in_tok = true;
        }

        if (in_tok) {
            if (in_escape) {
                tok += chr;
                in_escape = false;
            } else if (chr == in_quote) {
                in_quote = 0;
                toks.push_back(tok);
                tok = "";
                in_tok = false;
            } else if (chr == '\\') {
                in_escape = true;
            } else if (in_quote) {
                tok += chr;
            } else if (chr == '"' || chr == '\'') {
                in_quote = chr;
            } else if (isspace(chr)) {
                toks.push_back(tok);
                tok = "";
                in_tok = false;
            } else {
                tok += chr;
            }
        }
    }

    if (in_tok && tok.size()) {
        toks.push_back(tok);
    }

    return toks.size() - start_size;
}

/**
 * Un-escape backslash escaped content in string, inserting newlines etc for
 * known special characters.
 */
std::string plux::str_unescape(const std::string& src, size_t pos, size_t len)
{
    std::string dst;
    bool in_escape = false;
    size_t end = std::min(pos + len, src.size());
    for (; pos < end; ++pos) {
        char chr = src[pos];
        if (in_escape) {
            in_escape = false;
            switch (chr) {
            case 'n':
                dst += '\n';
                break;
            case 't':
                dst += '\t';
                break;
            case 'r':
                dst += '\r';
                break;
            default:
                dst += chr;
                break;
            }
        } else if (chr == '\\') {
            in_escape = true;
        } else {
            dst += chr;
        }
    }
    return dst;
}

/**
 * Scan for end in str, excluding quoted and escaped data.
 */
size_t plux::str_scan(const std::string& str, size_t pos,
                      const std::string& end)
{
    bool in_escape = false;
    char in_quote = '\0';
    for (; pos < str.size(); ++pos) {
        char chr = str[pos];
        if (in_escape) {
            in_escape = false;
        } else if (chr == '\\') {
            in_escape = true;
        } else if (chr == in_quote) {
            in_quote = '\0';
        } else if (chr == '"' || chr == '\'') {
            in_quote = chr;
        } else if (! in_quote && chr == end[0]) {
            if (str_view(str, pos, end.size()) == end) {
                return pos;
            }
        }
    }
    return -1;
}

plux::sview plux::str_view(const std::string& str, size_t pos, size_t len)
{
    if (str.size() <= pos) {
        return sview("", 0);
    }
    size_t avail = str.size() - pos;
    return sview(str.c_str() + pos, std::min(avail, len));
}
