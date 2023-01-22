project(crocconvert)

add_executable(crocconvert
        main.c

        crocconvert.c
        brender.c
        material.c
        pixelmap.c
        wad.c
        track.c
        model.c

        texconv.c
        savetga.c
        savetga.h

        br_wadfs.c
        cccc.c
        defs.h
        )

target_link_libraries(crocconvert PRIVATE libcroc vsclib vscpplib BRender::Core)

set_target_properties(crocconvert PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE

        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        )