#include <cstdlib>

#include "cfg.hh"
#include "os.hh"

Cfg::Cfg(void)
    : _log_dir("plux"),
      _stdlib_dir(PLUX_STDLIB_PATH)
{
    plux::os_ensure_dir(_log_dir);
    const char* env_stdlib_dir = getenv("PLUX_STDLIB_PATH");
    if (env_stdlib_dir != nullptr) {
        _stdlib_dir = env_stdlib_dir;
    }
}

Cfg::~Cfg(void)
{
}
