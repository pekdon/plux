#pragma once

#include <string>

namespace plux
{
    bool os_ensure_dir(const std::string path, int mode=0750);
}
