#include "os.hh"

extern "C" {
#include <errno.h>
#include <sys/stat.h>
}

namespace plux
{
    bool os_ensure_dir(const std::string path, int mode)
    {
        int ret = mkdir(path.c_str(), mode);
        if (ret == -1) {
            return errno == EEXIST;
        }
        return true;
    }
}
