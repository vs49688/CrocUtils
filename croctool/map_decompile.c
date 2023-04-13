/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <string.h>
#include <cJSON.h>
#include <libcroc/map.h>
#include <libcroc/util.h>

int map_decompile(int argc, char **argv)
{
    FILE *fp = NULL;
    cJSON *j = NULL;
    char *s = NULL;
    CrocMap map;
    int ret = 1;

    if(argc != 2 && argc != 3)
        return 2;

    croc_map_init(&map);

    if((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if(croc_map_read(fp, &map) < 0) {
        fprintf(stderr, "Error reading map: %s\n", strerror(errno));
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    (void)croc_extract_level_info(argv[1], &map._level, &map._sublevel);

    if(!(j = croc_map_write_json(&map))) {
        fprintf(stderr, "JSON conversion failed: %s\n", strerror(errno));
        goto done;
    }

    if(!(s = cJSON_Print(j))) {
        fprintf(stderr, "String conversion failed: %s\n", strerror(errno));
        goto done;
    }

    if(argc == 3 && strcmp("-", argv[2]) != 0) {
        fp = fopen(argv[2], "w");
    } else {
        fp = stdout;
    }

    if(fp == NULL) {
        fprintf(stderr, "Unable to open output file '%s': %s\n", argv[2], strerror(errno));
        goto done;
    }

    if(fputs(s, fp) == EOF) {
        fprintf(stderr, "Unable to write output file.\n");
        goto done;
    }

    ret = 0;

done:
    if(s != NULL)
        cJSON_free(s);

    if(j != NULL)
        cJSON_Delete(j);

    croc_map_free(&map);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}
