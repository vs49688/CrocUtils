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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/wad.h>

int wad_extract(int argc, char **argv)
{
    const char *basename, *filename;
    char *tmpname = NULL;
    int ret = -1;
    FILE *fp = NULL;
    CrocWadEntry *index = NULL;
    size_t fcount = 0, i;
    void *data = NULL;

    if(argc != 3 && argc != 4)
        return 2;

    basename = argv[1];
    filename = argv[2];

    if((tmpname = vsc_asprintf("%s.idx", basename)) == NULL) {
        fprintf(stderr, "Failed to allocate memory: %s\n", strerror(errno));
        goto done;
    }

    if((fp = vsc_fopen(tmpname, "rb")) == NULL) {
        fprintf(stderr, "Unable to open index file '%s': %s\n", tmpname, strerror(errno));
        goto done;
    }

    if((index = croc_wad_read_index(fp, &fcount)) == NULL) {
        fprintf(stderr, "Unable to read index: %s\n", strerror(errno));
        goto done;
    }

    fclose(fp);
    fp = NULL;

    for(i = 0; i < fcount; ++i) {
        if(!strcmp(filename, index[i].filename))
            break;
    }

    if(i == fcount) {
        fprintf(stderr, "No such file '%s' in index.\n", filename);
        goto done;
    }

    sprintf(tmpname, "%s.wad", basename); /* This is safe. */

    if((fp = vsc_fopen(tmpname, "rb")) == NULL) {
        fprintf(stderr, "Unable to open wad file '%s': %s\n", tmpname, strerror(errno));
        goto done;
    }

    if((data = croc_wad_load_entry(fp, index + i)) == NULL) {
        fprintf(stderr, "Decompression failed: %s\n", strerror(errno));
        goto done;
    }

    fclose(fp);
    fp = NULL;

    free(tmpname);
    tmpname = NULL;

    if(argc == 4 && !strcmp("-", argv[3])) {
        fp = stdout;
    } else {
        if(argc == 4) {
            if(vsc_chdir(argv[3]) < 0) {
                fprintf(stderr, "Unable to open directory '%s': %s\n", argv[3], strerror(errno));
                goto done;
            }
        }

        if((fp = fopen(index[i].filename, "wb")) == NULL) {
            fprintf(stderr, "Unable to open output file '%s': %s\n", index[i].filename, strerror(errno));
            goto done;
        }
    }

    if(fwrite(data, index[i].uncompressed_size, 1, fp) != 1) {
        fprintf(stderr, "Unable to write output file: %s\n", strerror(EIO));
        goto done;
    }

    ret = 0;
done:

    if(data)
        free(data);

    if(index != NULL)
        croc_wad_free_index(index, fcount);

    if(tmpname != NULL)
        free(tmpname);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}
