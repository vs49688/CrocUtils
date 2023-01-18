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

int wad_extract_all(int argc, char **argv)
{
    const char *basename;
    char *tmpname = NULL;
    int ret = -1, r;
    FILE *fp = NULL, *fp2 = NULL;
    CrocWadEntry *index = NULL;
    size_t fcount = 0;
    void *data = NULL;

    if(argc != 2 && argc != 3)
        return 2;

    basename = argv[1];

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

    sprintf(tmpname, "%s.wad", basename); /* This is safe. */

    if((r = vsc_fopen(tmpname, "rb", &fp)) < 0) {
        vsc_fperror(stderr, r, "Unable to open wad file '%s'", tmpname);
        goto done;
    }

    if(argc == 3 && (r = vsc_chdir(argv[2])) < 0) {
        vsc_fperror(stderr, r, "Unable to open directory '%s'", argv[2]);
        goto done;
    }

    for(size_t i = 0; i < fcount; ++i) {
        const CrocWadEntry *e = index + i;

        if((r = vsc_fopen(e->filename, "wb", &fp2)) < 0) {
            vsc_fperror(stderr, r, "Unable to open output file '%s'", e->filename);
            goto next;
        }

        if((data = croc_wad_load_entry(fp, e)) == NULL) {
            vsc_fperror(stderr, VSC_ERROR(errno), "Decompression of '%s' failed", e->filename);
            goto next;
        }

        if(fwrite(data, e->uncompressed_size, 1, fp2) != 1) {
            vsc_fperror(stderr, VSC_ERROR(EIO), "Unable to write output file '%s'", e->filename);
            goto next;
        }

next:
        if(data != NULL)
            free(data);
        data = NULL;

        if(fp2 != NULL)
            fclose(fp2);
    }

    ret = 0;
done:

    if(index != NULL)
        croc_wad_free_index(index, fcount);

    if(tmpname != NULL)
        vsc_free(tmpname);

    if(fp != NULL)
        (void)fclose(fp);

    return ret;
}
