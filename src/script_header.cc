#include "script_header.hh"

namespace plux {

    // HeaderConfigRequire

    LineRes HeaderConfigRequire::run(ShellCtx& ctx, ShellEnv& env)
    {
        std::string env_val;
        if (env.get_env("", _key, env_val)
            && (_val.empty() || _val == env_val)) {
            return LineRes(RES_OK);
        }
        return LineRes(RES_ERROR);
    }

    std::string HeaderConfigRequire::to_string(void) const
    {
        if (_val.empty()) {
            return "HeaderConfigRequire " + _key;
        } else {
            return "HeaderConfigRequire " + _key + "=" + _val;
        }
    }

    // HeaderConfigSet

    LineRes HeaderConfigSet::run(ShellCtx& ctx, ShellEnv& env)
    {
        return LineRes(RES_SET, "", {_key, _val});
    }

    std::string HeaderConfigSet::to_string(void) const
    {
        return "HeaderConfigSet " + _key + "=" + _val;
    }

    // HeaderInclude

    LineRes HeaderInclude::run(ShellCtx& ctx, ShellEnv& env)
    {
        return LineRes(RES_INCLUDE, _include_file);
    }

    std::string HeaderInclude::to_string(void) const
    {
        return "HeaderInclude " + _include_file;
    }

}
