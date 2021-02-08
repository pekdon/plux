#ifndef _TIMEOUT_HH_
#define _TIMEOUT_HH_

extern "C" {
#include <time.h>
}    

namespace plux {

    class Timeout {
    public:
        Timeout(unsigned int timeout_ms);

        void restart();

        void set_timeout_ms(unsigned int timeout_ms);
        unsigned int get_ms_until_timeout();

    private:
        /** Timer start, set on restart() */
        struct timespec _start;
        /** Timeout. */
        struct timespec _timeout;
    };
    
};

#endif // _TIMEOUT_HH_
