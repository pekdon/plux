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
    const std::string& stdlib_dir(void) const { return _stdlib_dir; }

private:
    /** Path to log files for current run. */
    std::string _log_dir;
    /** Path to stdlib include files. */
    std::string _stdlib_dir;
};
