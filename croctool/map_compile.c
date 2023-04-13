/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/map.h>

int map_compile(int argc, char **argv)
{
    FILE *fp = NULL;
    void *ptr = NULL;
    size_t len = 0;
    cJSON *j = NULL;
    int ret = 1, r;
    CrocMap map;

    if(argc != 3)
        return 2;

    croc_map_init(&map);

    if(strcmp("-", argv[1]) == 0)
        fp = stdin;
    else
        fp = fopen(argv[1], "rb");

    if(fp == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        goto done;
    }

    if((r = vsc_freadall(&ptr, &len, fp)) < 0) {
        vsc_fperror(stderr, r, "Error reading input file");
        goto done;
    }

    if(fp != stdin)
        (void)fclose(fp);

    fp = NULL;

    if((j = cJSON_ParseWithLength(ptr, len)) == NULL) {
        fputs("JSON Parse Error\n", stderr);
        goto done;
    }

    vsc_free(ptr);
    ptr = NULL;

    if(croc_map_read_json(j, &map) == NULL) {
        fputs("Invalid map structure\n", stderr);
        goto done;
    }

    if((fp = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "Unable to open output file '%s': %s\n", argv[2], strerror(errno));
        goto done;
    }

    if(croc_map_write(fp, &map) < 0) {
        fprintf(stderr, "Unable to write output file: %s\n", strerror(errno));
        goto done;
    }

    ret = 0;

done:

    croc_map_free(&map);

    if(j != NULL)
        cJSON_Delete(j);

    if(ptr != NULL)
        vsc_free(ptr);

    if(fp != NULL && fp != stdin)
        (void)fclose(fp);

    return ret;
}
