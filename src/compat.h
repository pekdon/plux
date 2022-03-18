#ifndef _COMPAT_H_
#define _COMPAT_H_

#include <time.h>
#include <termios.h>

#include "config.h"

#ifndef HAVE_FORKPTY

pid_t forkpty(int *amaster, char *name,
              struct termios *termp, struct winsize *winp);

#endif // !HAVE_FORKPTY

#ifndef HAVE_TIMESPECCMP

#define timespeccmp(a, b, CMP)                  \
    (((a)->tv_sec == (b)->tv_sec)               \
     ? ((a)->tv_nsec CMP (b)->tv_nsec)          \
     : ((a)->tv_sec CMP (b)->tv_sec))

#endif // !HAVE_TIMESPECCMP

#ifndef HAVE_TIMESPECSUB
#define timespecsub(a, b, result)                           \
    do {                                                    \
        (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;       \
        (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;    \
        if ((result)->tv_nsec < 0) {                        \
            --(result)->tv_sec;                             \
            (result)->tv_nsec += 1000000000;                \
        }                                                   \
    } while (0)
#endif // !HAVE_TIMESPECSUB

#endif // _COMPAT_H_
