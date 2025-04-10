cmake_minimum_required(VERSION 3.0)
project(CrocUtils)

add_subdirectory(cJSON EXCLUDE_FROM_ALL)

set(VSCLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vsclib)
add_subdirectory(libcroc)
add_subdirectory(croctool)

install(TARGETS croctool DESTINATION bin)

option(LIBCROC_ENABLE_TESTS "Enable test applications" ON)

if(LIBCROC_ENABLE_TESTS AND NOT WIN32)
	add_subdirectory(tests)
endif()
