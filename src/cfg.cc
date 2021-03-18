#include "cfg.hh"

extern "C" {
#include <errno.h>
#include <sys/stat.h>
}

Cfg::Cfg(void)
    : _log_dir("plux")
{
    int ret = mkdir(_log_dir.c_str(), 0750);
    if (ret == -1) {
        if (errno != EEXIST) {
        }
    }
}

Cfg::~Cfg(void)
{
}
