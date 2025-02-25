noinst_LIBRARIES = libplux_lib.a
libplux_lib_a_SOURCES = \
    cfg.cc cfg.hh \
    compat.h \
    forkpty.cc \
    function.hh \
    line.cc line.hh \
    log.cc log.hh \
    output_format.cc output_format.hh \
    os.cc os.hh \
    plux.cc plux.hh \
    process.cc process.hh \
    process_base.cc process_base.hh \
    regex.cc regex.hh \
    script.cc script.hh \
    script_env.cc script_env.hh \
    script_header.cc script_header.hh \
    script_parse.cc script_parse.hh \
    script_run.cc script_run.hh \
    shell.cc shell.hh \
    shell_ctx.cc shell_ctx.hh \
    shell_log.cc shell_log.hh \
    str.cc str.hh \
    timeout.cc timeout.hh
libplux_lib_a_CXXFLAGS = -I../stdlib

bin_PROGRAMS = plux
plux_SOURCES = main.cc
plux_LDADD = libplux_lib.a

EXTRA_DIST = CMakeLists.txt
