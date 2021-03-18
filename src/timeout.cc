extern "C" {
#include <sys/time.h>
#include <assert.h>
#include "compat.h"
}

#include "timeout.hh"

namespace plux
{
    /**
     * Construct a new Timeout timer with timeout_ms timeout.
     */
    Timeout::Timeout(unsigned int timeout_ms)
    {
        set_timeout_ms(timeout_ms);
        restart();
    }

    /**
     * Restart timeout from now until timeout_ms in the future.
     */
    void Timeout::restart(void)
    {
        int ret = clock_gettime(CLOCK_MONOTONIC, &_start);
        assert(ret == 0);
    }

    /**
     * Set timeout_ms, affecting current timer and timer on restart.
     */
    void Timeout::set_timeout_ms(unsigned int timeout_ms)
    {
        _timeout.tv_sec = timeout_ms / 1000;
        _timeout.tv_nsec = (timeout_ms % 1000) * 1000000;
    }

    /**
     * Get milliseconds until current timer times out.
     */
    unsigned int Timeout::get_ms_until_timeout(void)
    {
        struct timespec now, elapsed, left;
        int ret = clock_gettime(CLOCK_MONOTONIC, &now);
        assert(ret == 0);
        timespecsub(&now, &_start, &elapsed);
        if (timespeccmp(&elapsed, &_timeout, >)) {
            return 0;
        }
        timespecsub(&_timeout, &elapsed, &left);
        return left.tv_sec * 1000 + left.tv_nsec / 1000000;
    }
}
