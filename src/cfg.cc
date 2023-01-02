#include <cstdlib>

#include "cfg.hh"

extern "C" {
#include <errno.h>
#include <sys/stat.h>
}

Cfg::Cfg(void)
    : _log_dir("plux"),
      _stdlib_dir(PLUX_STDLIB_PATH)
{
    int ret = mkdir(_log_dir.c_str(), 0750);
    if (ret == -1) {
        if (errno != EEXIST) {
        }
    }

    const char* env_stdlib_dir = getenv("PLUX_STDLIB_PATH");
    if (env_stdlib_dir != nullptr) {
        _stdlib_dir = env_stdlib_dir;
    }
}

Cfg::~Cfg(void)
{
}
