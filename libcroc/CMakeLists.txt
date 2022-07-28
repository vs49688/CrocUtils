project(libcroc)

add_library(libcroc STATIC
		checksum.c
		map.c
		map_read.c
		map_write.c
		map_json_read.c
		map_json_write.c

		wad.c
		util.c
		dir.c
		vec.c
		col.c
		mod.c
		texture.c
		chunk.c
		psx.c

		wadc.cpp

		include/libcroc.h
		include/libcroc/utildef.h
		include/libcroc/util.h
		include/libcroc/fixeddef.h
		include/libcroc/fixed.h
		include/libcroc/mapdef.h
		include/libcroc/map.h
		include/libcroc/checksum.h
		include/libcroc/wad.h
		include/libcroc/waddef.h
		include/libcroc/dir.h
		include/libcroc/dirdef.h
		include/libcroc/vec.h
		include/libcroc/vecdef.h
		include/libcroc/col.h
		include/libcroc/coldef.h
		include/libcroc/mod.h
		include/libcroc/moddef.h
		include/libcroc/tex.h
		include/libcroc/texdef.h
		include/libcroc/chunk.h
		include/libcroc/chunkdef.h

		include/libcroc/strun.h
		include/libcroc/strun.h
		strun.c

		include/libcroc/psx.h
		include/libcroc/psxdef.h

)

target_include_directories(libcroc PUBLIC include)

target_link_libraries(libcroc PRIVATE vsclib)
target_link_libraries(libcroc PUBLIC  cjson)

if(MINGW)
	target_compile_definitions(libcroc PRIVATE __USE_MINGW_ANSI_STDIO)
endif()

if(MSVC)
	target_compile_definitions(libcroc PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Disable RTTI and exceptions for our template-only C++ code
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(libcroc PRIVATE
		$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti -fno-exceptions>
	)
endif()

set_target_properties(libcroc PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED TRUE
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED TRUE
)
