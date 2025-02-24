#include "config.h"

#include <cstring>
#include <cstdlib>
#include <iostream>

extern "C" {
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif // HAVE_PTY_H
#ifdef HAVE_UTIL_H
#include <util.h>
#endif // HAVE_UTIL_H
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif // HAVE_LIBUTIL_H
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif // HAVE_TERMIOS_H
}

#include "compat.h"
#include "shell.hh"

plux::Shell::Shell(Log& log,
                   ShellLog* shell_log,
                   ProgressLog& progress_log,
                   const std::string& name,
                   const std::string& command,
                   ShellEnv& shell_env)
    : ProcessBase(log, shell_log, progress_log, name, command, shell_env,
                  true /* trim_special */),
      _fd(-1)
{
    pid_t pid = forkpty(&_fd, nullptr, nullptr, nullptr);
    if (pid < 0) {
        log_and_throw_strerror("forkpty failed");
    }

    set_pid(pid);
    if (pid == 0) {
        _shell_env.set_os_env();
        execlp(command.c_str(), command.c_str(), nullptr);
        _log << "Shell failed to exec shell " << command << ": "
             << strerror(errno) << LOG_LEVEL_ERROR;
        exit(1);
    }

    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1) {
        stop_pid(true);
        log_and_throw_strerror("failed to get flags from PTY fd");
    }
    int ret = fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
    if (ret == -1) {
        log_and_throw_strerror("failed to set O_NONBLOCK flag on PTY fd");
    }
}

plux::Shell::~Shell(void)
{
    Shell::stop();
    stop_pid(true);
}

void plux::Shell::set_alive(bool alive, int exitstatus)
{
    _is_alive = alive;
    _exitstatus = exitstatus;
}

int plux::Shell::fd_input() const
{
    return _fd;
}

int plux::Shell::fd_output() const
{
    return _fd;
}

void plux::Shell::stop()
{
    if (_fd != -1) {
        // Ctrl-C (signal ongoing command)
        write(_fd, "\003", 1);
        // Ctrl-D (try to exit the shell)
        write(_fd, "\004", 1);
        // Close and then kill with signal
        close(_fd);
        _fd = -1;
    }
}
