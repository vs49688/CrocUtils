/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
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
    int ret = -1, r;
    FILE *fp = NULL;
    const CrocWadEntry *index = NULL;
    CrocWadFs *wadfs = NULL;
    void *data = NULL;

    if(argc != 3 && argc != 4)
        return 2;

    basename = argv[1];
    filename = argv[2];

    if((r = croc_wadfs_open(&wadfs, basename)) < 0) {
        vsc_fperror(stderr, r, "Failed to open WAD");
        return -1;
    }

    r = croc_wadfs_load(wadfs, filename, &data, &index);
    if(r == VSC_ERROR(ENOENT)) {
        fprintf(stderr, "No such file '%s' in index.\n", filename);
        goto done;
    } else if(r < 0) {
        vsc_fperror(stderr, r, "Failed to load %s", filename);
        goto done;
    }

    if(argc == 4 && !strcmp("-", argv[3])) {
        fp = stdout;
    } else {
        if(argc == 4) {
            if((r = vsc_chdir(argv[3])) < 0) {
                vsc_fperror(stderr, r, "Unable to open directory '%s'", argv[3]);
                goto done;
            }
        }

        if((r = vsc_fopen(index->filename, "wb", &fp)) < 0) {
            vsc_fperror(stderr, r, "Unable to open output file '%s'", index->filename);
            goto done;
        }
    }

    if(fwrite(data, index->uncompressed_size, 1, fp) != 1) {
        vsc_fperror(stderr, VSC_ERROR(EIO), "Unable to write output file");
        goto done;
    }

    ret = 0;
done:

    if(data)
        vsc_free(data);

    if(wadfs != NULL)
        croc_wadfs_close(wadfs);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}
