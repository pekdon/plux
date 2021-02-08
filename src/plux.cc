#include "plux.hh"

namespace plux {

    const unsigned int default_timeout_ms = 60000;
    const std::string empty_string;

    const std::map<std::string, std::string> default_env = {
        {"_TAB_", "\t"},
        {"_CTRL_C_", "\003"}
    };
};
