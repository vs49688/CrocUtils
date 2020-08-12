project(libcroc)

add_library(libcroc STATIC
		checksum.c
		map.c
		map_read.c
		map_write.c
		map_json_read.c
		map_json_write.c

		wad.c

		include/libcroc.h
)

target_include_directories(libcroc PUBLIC include)

target_link_libraries(libcroc PRIVATE vsclib)
target_link_libraries(libcroc PUBLIC  cjson)
