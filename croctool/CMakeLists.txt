project(croctool)

add_executable(croctool
		./main.c

		./map_decompile.c
		./map_compile.c
		./map_convert.c
		./map_unwad.c

		./crocfile_extract.c
		./crocfile_dump.c

		./mod_burst2obj.c
)

target_link_libraries(croctool libcroc vsclib)
