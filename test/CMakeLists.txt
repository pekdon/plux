set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(common_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src)
if (LIBUTIL)
    set(common_LIBRARIRES ${LIBUTIL})
endif (LIBUTIL)

add_executable(test_log test_log.cc)
add_test(log test_log)
set_target_properties(test_log PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_log PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_log libplux ${common_LIBRARIRES})

add_executable(test_plux test_plux.cc)
add_test(plux test_script_run)
set_target_properties(test_plux PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_plux PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_plux libplux ${common_LIBRARIRES})

add_executable(test_script test_script.cc)
add_test(script test_script)
set_target_properties(test_script PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_script PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_script libplux ${common_LIBRARIRES})

add_executable(test_script_parse test_script_parse.cc)
add_test(script_parse test_script_parse)
set_target_properties(test_script_parse PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_script_parse PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_script_parse libplux ${common_LIBRARIRES})

add_executable(test_script_run test_script_run.cc)
add_test(script_run test_script_run)
set_target_properties(test_script_run PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_script_run PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_script_run libplux ${common_LIBRARIRES})

add_executable(test_str test_str.cc)
add_test(str test_script_run)
set_target_properties(test_str PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_str PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_str libplux ${common_LIBRARIRES})

add_executable(test_timeout test_timeout.cc)
add_test(timeout test_timeout)
set_target_properties(test_timeout PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_timeout PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_timeout
  libplux ${common_LIBRARIRES})

add_executable(test_util test_util.cc)
add_test(str test_script_run)
set_target_properties(test_util PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON)
target_include_directories(test_util PUBLIC ${common_INCLUDE_DIRS})
target_link_libraries(test_util libplux ${common_LIBRARIRES})
