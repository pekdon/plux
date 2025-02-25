#pragma once

#include "process_base.hh"

namespace plux
{
    /**
     * Process control, used to run processes without any shell/terminal
     * in-between.
     */
    class Process : public ProcessBase {
    public:
        Process(Log& log,
                ShellLog* shell_log,
                ProgressLog& progress_log,
                const std::string& name,
                const std::vector<std::string>& args,
                ShellEnv& shell_env);
        Process(const Process& progress) = delete;
        virtual ~Process();

        void set_alive(bool alive, int exitstatus) override;

        int fd_input() const override;
        int fd_output() const override;
        void stop() override;

     private:
        void cleanup();

        int _stdin_pipe[2];
        int _stdout_pipe[2];
    };
}
