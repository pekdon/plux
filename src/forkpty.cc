#include "compat.h"

#ifndef HAVE_FORKPTY

#ifdef __sun
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/syscall.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> // grantpt
#include <string.h>
#include <signal.h>

static int _openpty_cleanup(int master, int slave)
{
    if (slave != -1) {
        close(slave);
    }
    if (master != -1) {
        close(master);
    }
    return -1;
}

int openpty(int *amaster, int *aslave, char *name,
        struct termios *termp, struct winsize *winp)
{
    int slave = -1;
    int master = open("/dev/ptmx", O_RDWR);
    if (master == -1) {
        return _openpty_cleanup(master, slave);
    }

    // grantpt will invoke a setuid program to change permissions
    // and might fail if SIGCHLD handler is set, temporarily reset
    // while running
    void(*sig_saved)(int) = signal(SIGCHLD, SIG_DFL);
    int res = grantpt(master);
    signal(SIGCHLD, sig_saved);

    if (res == -1 || unlockpt(master) == -1) {
        return _openpty_cleanup(master, slave);
    }

    char *slave_name = ptsname(master);
    if (slave_name == NULL) {
        return _openpty_cleanup(master, slave);
    }

    slave = open(slave_name, O_RDWR|O_NOCTTY);
    if (slave == -1) {
        return _openpty_cleanup(master, slave);
    }

    // ptem emulates a terminal when used on a pseudo terminal driver,
    // must be pushed before ldterm
    ioctl(slave, I_PUSH, "ptem");
    // ldterm provides most of the termio terminal interface
    ioctl(slave, I_PUSH, "ldterm");
    // ttcompat compatability with older terminal ioctls
    ioctl(slave, I_PUSH, "ttcompat");

    if (termp) {
        tcsetattr(slave, TCSAFLUSH, termp);
    }
    if (winp) {
        ioctl(slave, TIOCSWINSZ, winp);
    }

    *amaster = master;
    *aslave = slave;
    if (name) {
        strcpy(name, slave_name);
    }
    return 0;
}

int login_tty(int fd)
{
    setsid();
    if (ioctl(fd, TIOCSCTTY, NULL) == -1) {
        return -1;
    }

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    if (fd > STDERR_FILENO) {
        close(fd);
    }

    return 0;
}

pid_t forkpty(int *amaster, char *name,
              struct termios *termp, struct winsize *winp)
{
    int master, slave;
    if (openpty(&master, &slave, name, termp, winp) == -1) {
        return -1;
    }

    pid_t pid = fork();
    switch (pid) {
    case -1:
        close(master);
        close(slave);
        if (name) {
            name[0] = '\0';
        }
        return -1;
    case 0:
        close(master);
        login_tty(slave);
        return 0;
    default:
        close(slave);
        *amaster = master;
        return pid;
    }
}

#endif // __sun

#endif // !HAVE_FORKPTY
