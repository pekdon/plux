#include "shell_ctx.hh"

namespace plux
{
    UndefinedException::UndefinedException(const std::string& shell,
                                           const std::string& type,
                                           const std::string& name) throw()
        : _shell(shell),
          _type(type),
          _name(name)
    {
    }

    UndefinedException::~UndefinedException(void) throw()
    {
    }
}
