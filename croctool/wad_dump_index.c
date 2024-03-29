/*
 * CrocUtils - Copyright (C) 2021 Zane van Iperen.
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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/wad.h>

int wad_dump_index(int argc, char **argv)
{
    FILE *fp = NULL;
    cJSON *j = NULL;
    char *s = NULL;
    int ret = 1, r;
    CrocWadEntry *entries = NULL;
    size_t ecount = 0;

    if(argc != 2 && argc != 3)
        return 2;

    if((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Unable to open directory '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if((r = croc_wad_read_index(fp, &entries, &ecount)) < 0) {
        vsc_fperror(stderr, r, "Unable to read index");
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    if(!(j = croc_wad_index_write_json(entries, ecount))) {
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

    if(entries != NULL)
        vsc_free(entries);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}
