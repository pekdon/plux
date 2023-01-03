#pragma once

extern "C" {
#include <time.h>
}

namespace plux
{
    class Timeout {
    public:
        explicit Timeout(unsigned int timeout_ms);

        void restart(void);

        void set_timeout_ms(unsigned int timeout_ms);
        unsigned int get_ms_until_timeout(void);

    private:
        /** Timer start, set on restart() */
        struct timespec _start;
        /** Timeout. */
        struct timespec _timeout;
    };
}
