cmake_minimum_required(VERSION 3.0)
project(CrocUtils)

function(add_cjson)
	set(ENABLE_CJSON_TEST                OFF CACHE BOOL "" FORCE)
	set(ENABLE_TARGET_EXPORT             OFF CACHE BOOL "" FORCE)
	set(ENABLE_CJSON_UTILS               OFF CACHE BOOL "" FORCE)
	set(ENABLE_LOCALES                   OFF CACHE BOOL "" FORCE)
	set(ENABLE_CUSTOM_COMPILER_FLAGS     OFF CACHE BOOL "" FORCE)
	set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
	set(CJSON_BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
	add_subdirectory(cJSON EXCLUDE_FROM_ALL)

	# Until https://github.com/DaveGamble/cJSON/pull/488 is merged
	target_include_directories(cjson PUBLIC cJSON)
endfunction(add_cjson)


add_cjson()

set(VSCLIB_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vsclib)
add_subdirectory(libcroc)
add_subdirectory(croctool)

install(TARGETS croctool DESTINATION bin)

option(LIBCROC_ENABLE_TESTS "Enable test applications" ON)

if(LIBCROC_ENABLE_TESTS AND NOT WIN32)
	add_subdirectory(tests)
endif()
