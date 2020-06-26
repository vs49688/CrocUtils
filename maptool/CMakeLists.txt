project(maptool)

add_executable(maptool
		./main.c
		./map.c
		./map.h
		./json_write.c
		./json_read.c
		./unwad.c
		./checksum.c

		./cJSON.c
		./cJSON.h
)

target_link_libraries(maptool vsclib)
