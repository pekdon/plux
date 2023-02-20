#include <cstring>
#include <fstream>
#include <iostream>

#include "plux.hh"
#include "script_parse.hh"
#include "script_run.hh"

extern "C" {
#include <getopt.h>
#include <glob.h>
#include <signal.h>
}

extern char **environ;

static void signal_handler(int signal)
{
    switch (signal) {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
        break;
    }
}

static void fill_os_env(plux::env_map& env)
{
    for (char **cenv = environ; *cenv != 0; cenv++) {
        char *pos = strchr(*cenv, '=');
        if (pos == 0) {
            continue;
        }
        std::string key(*cenv, 0, pos - *cenv);
        std::string val(pos + 1);
        env[key] = val;
    }
}

static int usage(const char* name)
{
    std::cerr << "usage: " << name << " script" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    -d --dump" << std::endl;
    std::cerr << "    -h --help" << std::endl;
    std::cerr << "    -l --log-level" << std::endl;
    std::cerr << "    -t --tail" << std::endl;
    return 1;
}

static void dump_lines(plux::line_it it, plux::line_it end)
{
    for (; it != end; ++it) {
        std::cout << (*it)->file() << ":" << (*it)->line() << " "
                  << (*it)->shell() << " " << (*it)->to_string() << std::endl;
    }
}

static int dump_script(plux::Script* script)
{
    std::cout << "HEADERS" << std::endl;
    dump_lines(script->header_begin(), script->header_end());
    std::cout << std::endl << "SCRIPT" << std::endl;
    dump_lines(script->line_begin(), script->line_end());
    std::cout << std::endl << "CLEANUP" << std::endl;
    dump_lines(script->cleanup_begin(), script->cleanup_end());
    std::cout << std::endl << "FUNCTIONS" << std::endl;
    auto it = script->env().fun_begin();
    for (; it != script->env().fun_end(); ++it) {
        std::cout << std::endl << it->first << std::endl << std::endl;
        dump_lines(it->second->line_begin(), it->second->line_end());
    }
    return 0;
}

static int run_script(plux::Script* script, enum plux::log_level log_level,
                      bool tail, size_t n, size_t tot)
{
    int exitcode = 1;
    plux::LogFile log(log_level, "plux.log");
    plux::FileProgressLog progress_log("plux.progress.log");

    plux::env_map env;
    fill_os_env(env);
    auto run = plux::ScriptRun(log, progress_log, env, script, tail);
    std::cout << plux::format_timestamp() << ": " << script->file()
              << " (" << n << "/" << tot << ")" << std::endl;
    auto res = run.run();
    std::cout << plux::format_timestamp() << ": " << script->file() << ": ";
    if (res.status() == plux::RES_OK) {
        std::cout << "OK" << std::endl;
        exitcode = 0;
    } else {
        if (res.status() == plux::RES_TIMEOUT) {
            std::cout << "Timeout ";
        } else {
            std::cout << "Error ";
        }
        std::cout << res.error() << std::endl
                  << res.file() << ":" << res.linenumber()
                  << " " << res.error() << std::endl;
        auto it = res.stack_begin();
        for (; it != res.stack_end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    return exitcode;
}

static int run_file(int opt_dump, enum plux::log_level opt_log_level,
                    bool opt_tail, std::string file, size_t n, size_t tot)
{
    int exitcode = 1;

    std::filebuf fb;
    if (! fb.open(file, std::ios::in)) {
        std::cerr << "failed to open: " << file << std::endl;
        return exitcode;
    }

    std::istream is(&fb);
    try {
        plux::ScriptEnv script_env;
        plux::ScriptParse script_parse(file, &is, script_env);
        auto script = script_parse.parse();

        if (opt_dump) {
            exitcode = dump_script(script.get());
        } else {
            exitcode = run_script(script.get(), opt_log_level, opt_tail,
                                  n, tot);
        }
    } catch (plux::ScriptParseError& ex) {
        std::cerr << "parsing of " << ex.path() << " failed at line "
                  << ex.linenumber() << std::endl
                  << "error: " << ex.error() << std::endl
                  << "content: " << ex.line() << std::endl;
    }

    return exitcode;
}

static int run_files(int opt_dump, enum plux::log_level opt_log_level,
                     bool opt_tail, std::vector<std::string>& files)
{
    int exitcode = 0;
    std::vector<std::string>::iterator it(files.begin());
    for (size_t n = 1; it != files.end(); n++, ++it) {
        int file_exitcode = run_file(opt_dump, opt_log_level, opt_tail, *it,
                                     n, files.size());
        if (exitcode == 0 && file_exitcode) {
            exitcode = file_exitcode;
        }
    }
    return exitcode;
}

static void find_plux_files(int argc, char** argv,
                            std::vector<std::string>& files)
{

    for (int i = 0; i < argc; i++) {
        glob_t pglob = {0};
        if (glob(argv[i], 0, nullptr, &pglob)) {
            std::cerr << "glob " << argv[i] << " failed: " << strerror(errno)
                      << std::endl;
        } else {
            for (size_t j = 0; j < pglob.gl_pathc; j++) {
                files.push_back(pglob.gl_pathv[j]);
            }
            globfree(&pglob);
        }
    }
}

int main(int argc, char *argv[])
{
    const char* name = argv[0];

    struct option longopts[] = {
        {"dump", no_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {"log-level", required_argument, nullptr, 'l'},
        {"tail", no_argument, nullptr, 't'},
        {nullptr, no_argument, nullptr, '\0'}
    };

    bool opt_dump = false;
    bool opt_tail = false;
    enum plux::log_level opt_log_level = plux::LOG_LEVEL_INFO;

    int ch;
    while ((ch = getopt_long(argc, argv, "dhl:t", longopts, nullptr)) != -1) {
        switch (ch) {
        case 'd':
            opt_dump = true;
            break;
        case 'h':
            return usage(name);
            break;
        case 'l':
            opt_log_level = plux::level_from_string(optarg);
            if (opt_log_level == plux::LOG_LEVEL_NO) {
                return usage(name);
            }
            break;
        case 't':
            opt_tail = true;
            break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        return usage(name);
    }

    struct sigaction act;
    act.sa_handler = signal_handler;
    act.sa_mask = sigset_t();
    act.sa_flags = SA_NOCLDSTOP | SA_NODEFER;
    sigaction(SIGHUP, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGINT, &act, 0);

    std::vector<std::string> files;
    find_plux_files(argc, argv, files);

    return run_files(opt_dump, opt_log_level, opt_tail, files);
}
