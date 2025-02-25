#pragma once

#include "process_base.hh"

namespace plux
{
    /**
     * Interactive shell
     */
    class Shell : public ProcessBase {
    public:
        Shell(Log& log,
              ShellLog* shell_log,
              ProgressLog& progress_log,
              const std::string& name,
              const std::string& command,
              ShellEnv& shell_env);
        Shell(const Shell& shell) = delete;
        virtual ~Shell();

        void set_alive(bool alive, int exitstatus) override;

        int fd_input() const override;
        int fd_output() const override;
        void stop() override;

    private:
        int _fd;
    };
}
