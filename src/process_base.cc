#include "config.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <iostream>

extern "C" {
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
}

#include "compat.h"
#include "shell.hh"

plux::ShellException::ShellException(const std::string& shell,
                                     const std::string& error)
    : _shell(shell),
      _error(error)
{
}

plux::ProcessBase::ProcessBase(Log& log,
                               ShellLog* shell_log,
                               ProgressLog& progress_log,
                               const std::string& name,
                               const std::string& command,
                               ShellEnv& shell_env)
    : _log(log),
      _shell_log(shell_log),
      _progress_log(progress_log),
      _shell_env(shell_env),
      _is_alive(true),
      _exitstatus(-1),
      _name(name),
      _timeout_ms(plux::default_timeout_ms),
      _command(command),
      _buf_matched(false),
      _pid(-1)
{
}

int plux::ProcessBase::wait_pid(bool wait)
{
    int exitstatus = -1;
    if (_pid == -1) {
        return exitstatus;
    }

    int status;
    int flags = wait ? 0 : WNOHANG;
    // FIXME: wait timeout..
    if (_pid ==  waitpid(_pid, &status, flags)) {
        exitstatus = WEXITSTATUS(status);
        _log.trace("shell", "pid " + _pid_str + " finished with "
                   + std::to_string(exitstatus));
    }
    return exitstatus;
}

/**
 * Progress log message.
 */
void plux::ProcessBase::progress_log(const std::string& msg)
{
    std::cout << format_timestamp() << ": [" << _name << "] " << msg
              << std::endl;
    _progress_log.log(_name, msg);
}

/**
 * Progress log message with line context.
 */
void plux::ProcessBase::progress_log(const std::string& context,
                                     const std::string& msg)
{
    std::cout << format_timestamp() << " " << context << " [" << _name
              << "] " << msg << std::endl;
    _progress_log.log(_name, msg);
}

/**
 * Send input to shell.
 *
 * @return true if all data was written, else false.
 */
bool plux::ProcessBase::input(const std::string& data)
{
    _shell_log->input(data);

    size_t nleft = data.size();
    do {
        int ret = write(fd_output(), data.c_str(), data.size());
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

void plux::ProcessBase::output(const char* data, ssize_t size)
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

void plux::ProcessBase::line_consume_until(line_it it)
{
    _lines.erase(_lines.begin(), it);
}

void plux::ProcessBase::match_error(const std::string& line, bool is_line)
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

bool plux::ProcessBase::stop_pid(bool wait)
{
    if (_pid == -1) {
        return true;
    }

    if (kill(_pid, SIGKILL)) {
        if (errno != ESRCH) {
            _log << "failed to send SIGKILL to " << _pid_str << ": "
                 << strerror(errno) << LOG_LEVEL_ERROR;
        }
        return false;
    }

    _log << "sent SIGKILL to " << _pid_str << ", waiting for process to stop"
         << LOG_LEVEL_TRACE;
    return wait_pid(wait) == -1 ? false : true;
}

void plux::ProcessBase::log_and_throw_strerror(const std::string& msg)
{
    _log << "Shell" << msg << ": " << strerror(errno) << LOG_LEVEL_ERROR;
    throw ShellException(_name, msg);
}
