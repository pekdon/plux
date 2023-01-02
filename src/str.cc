#include "str.hh"

namespace plux
{
    /**
     * Split string into tokens supporting quotation.
     */
    size_t str_split(const std::string& str, std::vector<std::string>& toks)
    {
        bool in_tok = false;
        bool in_escape = false;
        char in_quote = 0;
        size_t start_size = toks.size();

        std::string tok;
        for (size_t pos = 0; pos < str.size(); ++pos) {
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
}
