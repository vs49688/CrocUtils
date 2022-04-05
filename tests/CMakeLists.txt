project(libcroc_tests)

add_executable(libcroc_tests
        main.cpp
        catch.hpp

        utils.cpp
        utils.hpp

        checksum.cpp
        colour.cpp
        directory.cpp
        map.cpp
)

target_link_libraries(libcroc_tests libcroc vsclib vscpplib)

set_target_properties(libcroc_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
