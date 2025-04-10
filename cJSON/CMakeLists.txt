project(cJSON)

add_library(cjson STATIC
        cJSON.c
        include/cJSON.h
)

target_include_directories(cjson PUBLIC include)
