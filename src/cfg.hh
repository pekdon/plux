#ifndef _CFG_HH_
#define _CFG_HH_

#include <string>

/**
 * plux runtime configuration.
 */
class Cfg {
public:
    Cfg();
    ~Cfg();

    const std::string& log_dir() const { return _log_dir; }

private:
    /** Path to log files for current run. */
    std::string _log_dir;
};

#endif // _CFG_HH_
