#include "process.hh"
#include "util.hh"

#include <cstring>
#include <memory>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

static void _init_pipe(int *fds)
{
    fds[0] = -1;
    fds[1] = -1;
}

static void _close_pipe(int *fds)
{
    if (fds[0] != -1) {
        close(fds[0]);
    }
    if (fds[1] != -1) {
        close(fds[1]);
    }
}

plux::Process::Process(Log& log,
                       ShellLog* shell_log,
                       ProgressLog& progress_log,
                       const std::string& name,
                       const std::vector<std::string>& args,
                       ShellEnv& shell_env)
    : ProcessBase(log, shell_log, progress_log, name, args[0], shell_env,
                  false /* trim_special */)
{
    _init_pipe(_stdin_pipe);
    _init_pipe(_stdout_pipe);

    Defer cleanup([this] { this->cleanup(); });

    if (pipe(_stdin_pipe) || pipe(_stdout_pipe)) {
        log_and_throw_strerror("pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        log_and_throw_strerror("fork");
    }
    set_pid(pid);


    if (pid == 0) {
        cleanup.cancel();

        // child
        dup2(_stdin_pipe[0], STDIN_FILENO);
        dup2(_stdout_pipe[1], STDOUT_FILENO);
        dup2(_stdout_pipe[1], STDERR_FILENO);

        _shell_env.set_os_env();
        char **argv = new char*[args.size() + 1];
        int i = 0;
        for (auto &arg : args) {
            argv[i++] = const_cast<char*>(arg.c_str());
        }
        argv[i] = nullptr;
        execvp(argv[0], argv);
        _log << "Process" << "failed to exec " << argv[0] << ": "
             << strerror(errno) << LOG_LEVEL_ERROR;
        delete [] argv;
        exit(127);
    }

    int flags = fcntl(fd_input(), F_GETFL, 0);
    if (flags == -1) {
        log_and_throw_strerror("failed to get flags from input fd");
    }
    int ret = fcntl(fd_input(), F_SETFL, flags | O_NONBLOCK);
    if (ret == -1) {
        log_and_throw_strerror("failed to set O_NONBLOCK input fd");
    }
    cleanup.cancel();
}

plux::Process::~Process()
{
    cleanup();
}

void plux::Process::cleanup()
{
    stop_pid(true);
    _close_pipe(_stdin_pipe);
    _close_pipe(_stdout_pipe);
}

void plux::Process::set_alive(bool alive, int exitstatus)
{
    _is_alive = alive;
    _exitstatus = exitstatus;

    // write PROCESS-EXIT: to stdout pipe for output/matching in general IO
    // loop.
    std::string msg("PROCESS-EXIT: " + std::to_string(exitstatus) + "\n");
    write(_stdout_pipe[1], msg.c_str(), msg.size());
}

int plux::Process::fd_input() const
{
    return _stdout_pipe[0];
}

int plux::Process::fd_output() const
{
    return _stdin_pipe[1];
}

void plux::Process::stop()
{
    stop_pid(true);
}
