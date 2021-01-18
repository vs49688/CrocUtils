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
		mod.c

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
		include/libcroc/mod.h
		include/libcroc/moddef.h
)

target_include_directories(libcroc PUBLIC include)

target_link_libraries(libcroc PRIVATE vsclib)
target_link_libraries(libcroc PUBLIC  cjson)

if(MSVC)
	target_compile_definitions(libcroc PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
