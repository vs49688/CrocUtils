project(libcroc)

add_library(libcroc STATIC
		# TODO: Consider making this standalone
		cJSON.h
		cJSON.c

		libcroc.h

		checksum.c
		map.c
		map_read.c
		map_write.c
		map_json_read.c
		map_json_write.c

		wad.c
)

target_link_libraries(libcroc PRIVATE vsclib)
target_include_directories(libcroc PUBLIC .)
