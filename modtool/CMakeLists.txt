project(modtool)

add_executable(modtool
		./main.c
)

target_link_libraries(modtool libcroc vsclib)
