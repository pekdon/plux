if TESTS
noinst_PROGRAMS = test_log \
		  test_regex \
		  test_str \
		  test_util \
		  test_plux \
		  test_script \
		  test_script_parse \
		  test_script_run \
		  test_timeout

test_log_SOURCES = test_log.cc
test_log_CXXFLAGS = -I../src
test_log_LDADD = ../src/libplux_lib.a

test_regex_SOURCES = test_regex.cc
test_regex_CXXFLAGS = -I../src
test_regex_LDADD = ../src/libplux_lib.a

test_str_SOURCES = test_str.cc
test_str_CXXFLAGS = -I../src
test_str_LDADD = ../src/libplux_lib.a

test_util_SOURCES = test_util.cc
test_util_CXXFLAGS = -I../src
test_util_LDADD = ../src/libplux_lib.a

test_plux_SOURCES = test_plux.cc
test_plux_CXXFLAGS = -I../src
test_plux_LDADD = ../src/libplux_lib.a

test_script_SOURCES = test_script.cc
test_script_CXXFLAGS = -I../src
test_script_LDADD = ../src/libplux_lib.a

test_script_parse_SOURCES = test_script_parse.cc
test_script_parse_CXXFLAGS = -I../src
test_script_parse_LDADD = ../src/libplux_lib.a

test_script_run_SOURCES = test_script_run.cc
test_script_run_CXXFLAGS = -I../src
test_script_run_LDADD = ../src/libplux_lib.a

test_timeout_SOURCES = test_timeout.cc
test_timeout_CXXFLAGS = -I../src
test_timeout_LDADD = ../src/libplux_lib.a
endif

SUBDIRS = system

EXTRA_DIST = CMakeLists.txt \
	     plux.plux \
	     test.hh \
	     test_log.cc \
	     test_plux.cc \
	     test_script.cc \
	     test_script_parse.cc \
	     test_script_run.cc \
	     test_str.cc \
	     test_timeout.cc
