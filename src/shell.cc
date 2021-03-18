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

#include "shell.hh"

namespace plux
{
    ShellException::ShellException(const std::string& shell,
                                   const std::string& error) throw()
        : _shell(shell),
          _error(error)
    {
    }

    ShellException::~ShellException(void) throw()
    {
    }

    Shell::Shell(Log& log,
                 ShellLog* shell_log,
                 ProgressLog&,
                 const std::string& name,
                 const std::string& command,
                 ShellEnv& shell_env)
        : _log(log),
          _shell_log(shell_log),
          _name(name),
          _timeout_ms(plux::default_timeout_ms),
          _command(command),
          _shell_env(shell_env),
          _buf_matched(false),
          _fd(-1),
          _pid(-1)
    {
        int master;
        pid_t pid = forkpty(&master, nullptr /* name */,
                            nullptr /* termp */, nullptr /* winp */);
        if (pid < 0) {
            log_and_throw_strerror("forkpty failed");
        }

        if (pid == 0) {
            env_map_const_it it(_shell_env.os_begin());
            for (; it != _shell_env.os_end(); ++it) {
                setenv(it->first.c_str(), it->second.c_str(),
                       1 /* overwrite */);
            }

            execlp(command.c_str(), command.c_str(), nullptr);
            _log << "Shell" << "failed to exec shell " << command << ": "
                 << strerror(errno) << LOG_LEVEL_ERROR;
            exit(1);
        }

        int flags = fcntl(master, F_GETFL, 0);
        if (flags == -1) {
            stop_pid(pid);
            log_and_throw_strerror("failed to get flags from PTY fd");
        }
        int ret = fcntl(master, F_SETFL, flags | O_NONBLOCK);
        if (ret == -1) {
            log_and_throw_strerror("failed to set O_NONBLOCK flag on PTY fd");
        }

        _fd = master;
        _pid = pid;
    }

    Shell::~Shell(void)
    {
        stop();
        kill(_pid, SIGKILL);
        int status;
        auto pid = std::to_string(static_cast<int>(_pid));
        _log.trace("shell", "waiting for pid " + pid + " to finish");
        waitpid(_pid, &status, 0);
        _log.trace("shell", "pid " + std::to_string(static_cast<int>(_pid))
                   + " finished with " + std::to_string(WEXITSTATUS(status)));
    }

    void Shell::stop(void)
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

    /**
     * Send input to shell.
     *
     * @return true if all data was written, else false.
     */
    bool Shell::input(const std::string& data)
    {
        _shell_log->input(data);

        size_t nleft = data.size();
        do {
            int ret = write(_fd, data.c_str(), data.size());
            if (ret == -1) {
                if (errno == EAGAIN) {
                    // FIXME: add POLLOUT to wait_for_input for given
                    // shell.
                }
                return false;
            }
            nleft -= ret;
        } while (nleft > 0);

        return true;
    }

    void Shell::output(const char* data, ssize_t size)
    {
        _shell_log->output(data, size);

        for (ssize_t i = 0; i < size; i++) {
            if (data[i] == '\n') {
                if (! _buf.empty() && _buf[_buf.size() - 1] == '\r') {
                    _buf.erase(_buf.size() - 1);
                }

                match_error(_buf, true);

                if (! _buf_matched) {
                    _lines.push_back(_buf);
                }
                _buf = "";
                _buf_matched = false;
            } else {
                _buf += data[i];
            }
        }
        match_error(_buf, false);
    }

    void Shell::line_consume_until(line_it it)
    {
        _lines.erase(_lines.begin(), it);
    }

    void Shell::match_error(const std::string& line, bool is_line)
    {
        if (_error_pattern.empty()) {
            return;
        }
        if (! is_line && _error_pattern[_error_pattern.size() - 1] == '$') {
            return;
        }

        if (plux::regex_search(line, _error)) {
            throw ShellException(_name,
                                 std::string("error pattern ") +
                                 _error_pattern + " matched");
        }
    }

    void Shell::stop_pid(pid_t pid)
    {
        if (! kill(pid, SIGKILL)) {
            _log << "Shell" << "sent SIGKILL to " << pid << ", waiting for stop"
                 << LOG_LEVEL_INFO;
            // FIXME: wait timeout..
            int exitcode;
            waitpid(pid, &exitcode, WNOHANG);
        } else {
            _log << "Shell" << "failed to send SIGKILL to " << pid
                 << ": " << strerror(errno) << LOG_LEVEL_ERROR;
        }
    }

    void Shell::log_and_throw_strerror(std::string msg)
    {
        _log << "Shell" << msg << ": " << strerror(errno) << LOG_LEVEL_ERROR;
        throw ShellException(_name, msg);
    }
}
