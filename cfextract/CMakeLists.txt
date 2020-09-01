project(cfextract)

add_executable(cfextract
		./main.c
)

target_link_libraries(cfextract vsclib)
target_link_libraries(cfextract libcroc)
