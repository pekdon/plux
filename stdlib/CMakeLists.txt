cmake_minimum_required(VERSION 3.0)

file(GLOB pluxinc_files "${CMAKE_CURRENT_SOURCE_DIR}/*.pluxinc")

install(FILES
	${pluxinc_files}
	DESTINATION "${CMAKE_INSTALL_FULL_SHAREDSTATEDIR}/plux/stdlib")
