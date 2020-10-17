/*
 * CrocUtils - Copyright (C) 2020 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, and only
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
    int ret = 1;
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

    if(vsc_freadall(&ptr, &len, fp) < 0) {
        fprintf(stderr, "Error reading input file: %s\n", strerror(errno));
        goto done;
    }

    if(fp != stdin)
        (void)fclose(fp);

    fp = NULL;

    if((j = cJSON_ParseWithLength(ptr, len)) == NULL) {
        fputs("JSON Parse Error\n", stderr);
        goto done;
    }

    free(ptr);
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
        free(ptr);

    if(fp != NULL && fp != stdin)
        (void)fclose(fp);

    return ret;
}
