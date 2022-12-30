#pragma once

#include <string>

#include "function.hh"

namespace plux
{
    /**
     * Global script environment, shared between Script to get global function
     * scope etc.
     */
    class ScriptEnv {
    public:
        ScriptEnv();
        ~ScriptEnv();

        Function* fun_get(const std::string& name) const;
        void fun_set(const std::string& name, Function* fun);

        fun_it fun_begin(void) const { return _funs.begin(); }
        fun_it fun_end(void) const { return _funs.end(); }

    private:
        /** map from function name to function. */
        std::map<std::string, Function*> _funs;
    };
}
