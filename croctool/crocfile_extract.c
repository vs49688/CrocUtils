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
#include <stdlib.h>
#include <errno.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/dir.h>
#include <libcroc/util.h>

static int write_manifest(const char *path, const CrocDirEntry *entries, size_t count)
{
    FILE *f = NULL;
    cJSON *j = NULL;
    char *s = NULL;
    int ret = -1, errno_;

    if((j = croc_dir_write_json(entries, count)) == NULL) {
        errno = ENOMEM;
        goto done;
    }

    if((s = cJSON_Print(j)) == NULL) {
        errno = ENOMEM;
        goto done;
    }

    if((f = fopen(path, "wb")) == NULL) {
        goto done;
    }

    if(fwrite(s, strlen(s), 1, f) != 1) {
        errno = ENOMEM;
        goto done;
    }

    ret = 0;

done:
    errno_ = errno;

    if(f != NULL)
        fclose(f);

    if(s != NULL)
        cJSON_free(s);

    if(j != NULL)
        cJSON_Delete(j);

    errno = errno_;
    return ret;
}

int crocfile_extract(int argc, char **argv)
{
    int oldstyle, ret = 1, r;
    size_t dcount, datasize = 0;
    CrocDirEntry *entries = NULL;
    FILE *f = NULL;
    void *data = NULL;
    char *outpath = NULL, *filename;

    const char *dirpath = argv[1];
    const char *datapath = argv[2];
    const char *out = ".";

    if(argc == 4)
        out = argv[3];
    else if(argc != 3)
        return 2;

    if((f = fopen(dirpath, "rb")) == NULL) {
        fprintf(stderr, "Unable to open file '%s': %s\n", dirpath, strerror(errno));
        goto done;
    }

    if((r = croc_dir_read(f, &entries, &dcount, &oldstyle)) < 0) {
        vsc_fperror(stderr, r, "Unable to read directory");
        goto done;
    }

    (void)fclose(f);

    if((f = fopen(datapath, "rb")) == NULL) {
        fprintf(stderr, "Unable to open data file: %s\n", strerror(errno));
        goto done;
    }

    if((r = vsc_freadall(&data, &datasize, f)) < 0) {
        vsc_fperror(stderr, r, "Unable to read data file");
        goto done;
    }

    (void)fclose(f);
    f = NULL;


    if((outpath = vsc_asprintf("%s/manifest.json", out)) == NULL) {
        fprintf(stderr, "%s\n", strerror(ENOMEM));
        goto done;
    }

    if(write_manifest(outpath, entries, dcount) < 0) {
        fprintf(stderr, "Unable to write manifest: %s\n", strerror(errno));
        goto done;
    }

    filename = (char*)croc_util_get_filename(outpath);

    for(size_t i = 0; i < dcount; ++i) {
        CrocDirEntry *d = entries + i;

        if(d->offset >= datasize|| (d->offset + d->size) > datasize) {
            fprintf(stderr, "Extents for file %s out of range, skipping...\n", d->name);
            continue;
        }

        strncpy(filename, d->name, CROC_DIRENTRY_NAME_SIZE + 1);

        if((f = fopen(outpath, "wb")) == NULL) {
            fprintf(stderr, "Unable to open output file %s: %s\n", outpath, strerror(errno));
            goto done;
        }

        if(d->size > 0 && fwrite((uint8_t*)data + d->offset, d->size, 1, f) != 1) {
            errno = EIO;
            fprintf(stderr, "Unable to write output file %s: %s\n", outpath, strerror(errno));
            goto done;
        }

        (void)fclose(f);
        f = NULL;
    }

    ret = 0;

done:

    if(outpath != NULL)
        free(outpath);

    if(data != NULL)
        free(data);

    if(entries != NULL)
        vsc_free(entries);

    if(f != NULL)
        fclose(f);
    return ret;
}
