#include "shell_log.hh"

namespace plux {
    /**
     * Create file based output log for shells, opens input and output
     * log for writing.
     */
    FileShellLog::FileShellLog(const std::string& path)
    {
        _input.open(path + "_input.log");
        _output.open(path + "_output.log");
    }

    /**
     * Cleanup resources used by file based output log, close files.
     */
    FileShellLog::~FileShellLog()
    {
        _input.close();
        _output.close();
    }

    /**
     *g Write to input log.
     */
    void FileShellLog::input(const std::string& data)
    {
        _input << data;
        _input.flush();
    }

    /**
     * Write to output log.
     */
    void FileShellLog::output(const char* data, ssize_t size)
    {
        _output.write(data, size);
        _output.flush();
    }

    FileProgressLog::FileProgressLog(const std::string& path)
    {
    }

    FileProgressLog::~FileProgressLog()
    {
    }

    void FileProgressLog::log(const std::string& shell, const std::string& msg)
    {
    }

    void FileProgressLog::progress_start(const std::string& shell,
                                         const std::string& msg)
    {
    }

    void FileProgressLog::progress_stop(const std::string& shell,
                                        const std::string& msg)
    {
    }
}
