#include "script_env.hh"

namespace plux
{
    ScriptEnv::ScriptEnv()
    {
    }

    ScriptEnv::~ScriptEnv()
    {
        for (auto it : _funs) {
            delete it.second;
        }
    }

    Function* ScriptEnv::fun_get(const std::string& name) const
    {
        auto it = _funs.find(name);
        return it == _funs.end() ? nullptr : it->second;
    }

    void ScriptEnv::fun_set(const std::string& name, Function* fun)
    {
        delete fun_get(name);
        _funs[name] = fun;
    }
}
