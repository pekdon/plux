cmake_minimum_required(VERSION 3.0)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)

project(plux)

# Check for C++11
check_cxx_compiler_flag(-std=c++11 HAS_CXX11)
if (HAS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else (HAS_CXX11)
  message(FATAL_ERROR "Compiler with C++11 support is required")
endif (HAS_CXX11)

check_include_file(pty.h HAVE_PTY_H)
check_include_file(util.h HAVE_UTIL_H)
check_include_file(termios.h HAVE_TERMIOS_H)

find_library(LIBUTIL util)
if (NOT LIBUTIL)
  message(FATAL_ERROR, "libutil not found")
endif (NOT LIBUTIL)

check_function_exists(setenv HAVE_SETENV)
check_symbol_exists(timespecsub sys/time.h HAVE_TIMESPECCMP)
check_symbol_exists(timespecsub sys/time.h HAVE_TIMESPECSUB)

# Look for platform specific tools
find_program(SH sh /usr/xpg4/bin/sh /bin/sh /usr/xpg4/bin)

add_definitions(-DSH="${SH}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)

configure_file(${CMAKE_SOURCE_DIR}/CMake/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_subdirectory(src)

if (TESTS)
   enable_testing()
   add_subdirectory(test)
endif (TESTS)
