set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(common_INCLUDE_DIRS ${PROJECT_BINARY_DIR})

set(libplux_SOURCES
  cfg.cc
  log.cc
  plux.cc
  regex.cc
  script.cc
  script_parse.cc
  script_run.cc
  shell.cc
  shell_log.cc
  timeout.cc)

add_library(libplux STATIC ${libplux_SOURCES})
target_include_directories(libplux PUBLIC ${common_INCLUDE_DIRS})

set_target_properties(libplux PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)

add_executable(plux main.cc)
target_include_directories(plux PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(plux libplux ${LIBUTIL})

set_target_properties(plux PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)

install(TARGETS plux DESTINATION bin)

