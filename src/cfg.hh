#pragma once

#include <string>

/**
 * plux runtime configuration.
 */
class Cfg {
public:
    Cfg(void);
    ~Cfg(void);

    const std::string& log_dir(void) const { return _log_dir; }

private:
    /** Path to log files for current run. */
    std::string _log_dir;
};
