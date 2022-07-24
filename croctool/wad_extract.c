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
#include <assert.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/wad.h>

int wad_extract(int argc, char **argv)
{
    const char *basename, *filename;
    char *tmpname = NULL;
    int ret = -1, r;
    FILE *fp = NULL;
    CrocWadEntry *index = NULL;
    size_t fcount = 0, i;
    void *data = NULL;

    if(argc != 3 && argc != 4)
        return 2;

    basename = argv[1];
    filename = argv[2];

    if((tmpname = vsc_asprintf("%s.idx", basename)) == NULL) {
        vsc_fperror(stderr, VSC_ERROR(ENOMEM), "Failed to allocate memory");
        goto done;
    }

    if((r = vsc_fopen(tmpname, "rb", &fp)) < 0) {
        vsc_fperror(stderr, r, "Unable to open index file '%s'", tmpname);
        goto done;
    }

    if((index = croc_wad_read_index(fp, &fcount)) == NULL) {
        vsc_fperror(stderr, VSC_ERROR(errno), "Unable to read index");
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

    if((r = vsc_fopen(tmpname, "rb", &fp)) < 0) {
        vsc_fperror(stderr, r, "Unable to open wad file '%s'", tmpname);
        goto done;
    }

    if((data = croc_wad_load_entry(fp, index + i)) == NULL) {
        vsc_fperror(stderr, VSC_ERROR(errno), "Decompression failed");
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
            if((r = vsc_chdir(argv[3])) < 0) {
                vsc_fperror(stderr, r, "Unable to open directory '%s'", argv[3]);
                goto done;
            }
        }

        if((r = vsc_fopen(index[i].filename, "wb", &fp)) < 0) {
            vsc_fperror(stderr, r, "Unable to open output file '%s'", index[i].filename);
            goto done;
        }
    }

    if(fwrite(data, index[i].uncompressed_size, 1, fp) != 1) {
        vsc_fperror(stderr, VSC_ERROR(EIO), "Unable to write output file");
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
