project(maptool)

add_executable(maptool
		./main.c
		./unwad.c
)

target_link_libraries(maptool libcroc vsclib)
