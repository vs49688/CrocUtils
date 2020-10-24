cmake_minimum_required(VERSION 3.0)
project(CrocUtils)

function(add_cjson)
	set(ENABLE_CJSON_TEST                OFF CACHE BOOL "" FORCE)
	set(ENABLE_TARGET_EXPORT             OFF CACHE BOOL "" FORCE)
	set(ENABLE_CJSON_UTILS               OFF CACHE BOOL "" FORCE)
	set(ENABLE_LOCALES                   OFF CACHE BOOL "" FORCE)
	set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON  CACHE BOOL "" FORCE)
	set(CJSON_BUILD_SHARED_LIBS          OFF CACHE BOOL "" FORCE)
	add_subdirectory(cJSON EXCLUDE_FROM_ALL)

	# Until https://github.com/DaveGamble/cJSON/pull/488 is merged
	target_include_directories(cjson PUBLIC cJSON)
endfunction(add_cjson)


add_cjson()

add_subdirectory(vsclib)
add_subdirectory(libcroc)
add_subdirectory(croctool)

if(MINGW)
	target_compile_definitions(croctool PUBLIC __USE_MINGW_ANSI_STDIO)
	target_compile_definitions(libcroc  PUBLIC __USE_MINGW_ANSI_STDIO)
endif()

install(TARGETS croctool DESTINATION bin)
