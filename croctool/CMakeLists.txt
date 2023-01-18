project(croctool)

set(CROCTOOL_VERSION_STRING "dev"     CACHE STRING "Embedded Version String")
set(CROCTOOL_COMMIT_HASH    "unknown" CACHE STRING "unknown")

configure_file(config.h.in config.h)

add_executable(croctool
		./main.c
		${CMAKE_CURRENT_BINARY_DIR}/config.h
		./version.c

		./map_decompile.c
		./map_compile.c
		./map_convert.c
		./map_unwad.c

		./crocfile_extract.c
		./crocfile_dump.c
		./crocfile_build.c

		./strun_dump.c

		./mod_burst2obj.c

		./mat_toscr.c

		./wad_dump_index.c
		./wad_extract.c
		./wad_extract_all.c

		./tex_convert.c

		./stb_image_write.h
)

target_include_directories(croctool PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(croctool libcroc vsclib BRender::Core)

if(MINGW)
	target_compile_definitions(croctool PRIVATE __USE_MINGW_ANSI_STDIO)
endif()

if(MSVC)
	target_compile_definitions(croctool PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
