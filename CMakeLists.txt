cmake_minimum_required(VERSION 3.0)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)

project(plux)

# Check for C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

check_include_file(pty.h HAVE_PTY_H)
check_include_file(util.h HAVE_UTIL_H)
check_include_file(libutil.h HAVE_LIBUTIL_H)
check_include_file(termios.h HAVE_TERMIOS_H)

find_library(LIBUTIL util)

# setenv
check_function_exists(setenv HAVE_SETENV)

# forkpty may require -lutil
set(orig_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
if (LIBUTIL)
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${LIBUTIL})
endif (LIBUTIL)
check_function_exists(forkpty HAVE_FORKPTY)
set(CMAKE_REQUIRED_LIBRARIES ${orig_CMAKE_REQUIRED_LIBRARIES})

check_symbol_exists(timespecsub sys/time.h HAVE_TIMESPECCMP)
check_symbol_exists(timespecsub sys/time.h HAVE_TIMESPECSUB)

# Add work-around for missing -std=c++11 flag when running the
# check_cxx_source_runs tests
set(orig_CMAKE_CXX_FLAGS ${CMAKE_CXX11_FLAGS})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})

check_cxx_source_runs("
#include <regex>
int main() { std::regex(\"^[A-Za-z0-9_-]+$\"); }
" WORKING_CXX_REGEX)

set(CMAKE_CXX_FLAGS ${orig_CMAKE_CXX_FLAGS})

# Look for platform specific tools
find_program(SH sh /usr/xpg4/bin/sh /bin/sh /usr/xpg4/bin)

add_definitions(-DSH="${SH}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)

option(PEDANTIC "turn on strict compile-time warnings" OFF)
option(TESTS "include tests" OFF)

if (PEDANTIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
endif (PEDANTIC)

configure_file(${CMAKE_SOURCE_DIR}/CMake/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

if (TESTS)
   enable_testing()
   add_subdirectory(test)
endif (TESTS)
