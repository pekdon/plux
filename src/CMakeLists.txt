set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(common_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src
			${PROJECT_BINARY_DIR}
			${PROJECT_BINARY_DIR}/src)

file(GLOB pluxinc_files "${CMAKE_SOURCE_DIR}/stdlib/*.pluxinc")

add_custom_command(OUTPUT stdlib_builtins.hh
		   COMMAND "${CMAKE_SOURCE_DIR}/stdlib/gen_stdlib_builtins.sh"
		   DEPENDS ${pluxinc_files})
add_custom_target(generate_stdlib_builtins DEPENDS stdlib_builtins.hh)

set(libplux_SOURCES
  cfg.cc
  forkpty.cc
  log.cc
  line.cc
  plux.cc
  regex.cc
  script.cc
  script_env.cc
  script_header.cc
  script_parse.cc
  script_run.cc
  shell.cc
  shell_ctx.cc
  shell_log.cc
  str.cc
  timeout.cc)

add_library(libplux STATIC ${libplux_SOURCES})
add_dependencies(libplux generate_stdlib_builtins)
target_include_directories(libplux PUBLIC ${common_INCLUDE_DIRS})

set_target_properties(libplux PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)

set(plux_LIBS libplux)
if (LIBUTIL)
    set(plux_LIBS ${plux_LIBS} ${LIBUTIL})
endif (LIBUTIL)
add_executable(plux main.cc)
target_include_directories(plux PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(plux ${plux_LIBS})

set_target_properties(plux PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)

install(TARGETS plux DESTINATION bin)

