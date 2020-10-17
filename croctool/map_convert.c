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
#include <libcroc/map.h>

int map_convert(int argc, char **argv)
{
    FILE *fp = NULL;
    CrocMap map;
    int ret = 1;
    long rebase;
    const char *in, *out;

    if(argc == 3) {
        in     = argv[1];
        out    = argv[2];
        rebase = 0;
    } else if(argc == 5) {
        char *e = NULL;

        if(strcmp(argv[1], "--rebase") != 0)
            return 2;

        rebase = strtol(argv[2], &e, 10);

        if(errno != 0 || *e != '\0' || rebase < -899 || rebase > 899)
            return 2;

        in  = argv[3];
        out = argv[4];
    } else {
        return 2;
    }


    croc_map_init(&map);

    if((fp = fopen(in, "rb")) == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if(croc_map_read(fp, &map) < 0) {
        fprintf(stderr, "Error reading map: %s\n", strerror(errno));
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    if(croc_map_rebase(&map, (int)rebase) < 0) {
        fprintf(stderr, "Unable to rebase map.\n");
        goto done;
    }

    if((fp = fopen(out, "wb")) == NULL) {
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

    if(fp != NULL)
        (void)fclose(fp);

    return ret;
}
