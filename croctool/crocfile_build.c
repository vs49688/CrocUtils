/*
 * CrocUtils - Copyright (C) 2022 Zane van Iperen.
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
#include <vsclib.h>
#include <errno.h>
#include <stdio.h>
#include <libcroc/util.h>
#include <libcroc/dir.h>

/* TODO: Move this into vsclib */
static char toupper_ascii(char c)
{
    if(c >= 'a' && c <= 'z')
        return (char)(c - 32);
    return c;
}

static int write_file(const char *path, CrocDirEntry *entry, FILE *outfp, vsc_off_t pos)
{
    int        r;
    void       *data;
    size_t     size, namelen;
    FILE       *fp;
    const char *basename;
    char       namebuf[CROC_DIRENTRY_NAME_SIZE + 1] = {0};

    basename = croc_util_get_filename(path);

    /* Truncate and uppercase the filename */
    strncpy(namebuf, basename, CROC_DIRENTRY_NAME_SIZE);
    namelen = strlen(namebuf);
    for(size_t i = 0; i < namelen; ++i)
        namebuf[i] = toupper_ascii(namebuf[i]);

    printf("[%12s] ...", namebuf);

    if((r = vsc_fopen(path, "rb", &fp)) < 0) {
        vsc_fperror(stdout, r, "\r[%12s] FAILED - fopen()\n", namebuf);
        return -1;
    }

    if((r = vsc_freadall(&data, &size, fp)) < 0) {
        vsc_fperror(stdout, r, "\r[%12s] FAILED - vsc_freadall()\n", namebuf);
        fclose(fp);
        return -1;
    }

    if(size > CROC_DIRENTRY_MAX_FILE_SIZE) {
        printf("\r[%12s] %zu bytes, too big, skipping...\n", namebuf, size);
        fclose(fp);
        vsc_free(data);
        return -1;
    }

    printf("\r[%12s] %zu bytes\n", namebuf, size);

    if(fwrite(data, size, 1, outfp) != 1) {
        fclose(fp);
        vsc_free(data);
        printf("\r[%12s] %zu bytes, fwrite() error, skipping...\n", namebuf, size);
        return -1;
    }

    fclose(fp);
    vsc_free(data);

    memcpy(entry->name, namebuf, sizeof(entry->name));
    entry->size    = (uint32_t)size;
    entry->restart = 0;
    entry->offset  = pos;
    entry->usage   = 0;
    return 0;
}

int crocfile_build(int argc, char **argv)
{
    CrocDirEntry *directory;
    const char   *dir_name, *data_name;
    size_t       file_count;
    FILE         *fp = NULL;
    vsc_off_t    pos;
    int          ret = -1, r;

    if(argc < 4)
        return 2;

    dir_name   = argv[1];
    data_name  = argv[2];
    file_count = argc - 3;

    if((directory = vsc_calloc(file_count, sizeof(CrocDirEntry))) == NULL) {
        vsc_fperror(stderr, VSC_ERROR(ENOMEM), "Failed to allocate memory");
        goto done;
    }

    if((r = vsc_fopen(data_name, "wb", &fp)) < 0) {
        vsc_fperror(stderr, r, "Unable to open file '%s'", data_name);
        goto done;
    }


    pos = 0;
    for(size_t i = 0; i < file_count; ++i) {
        CrocDirEntry *entry = directory + i;
        if(write_file(argv[3 + i], entry, fp, pos) < 0)
            goto done;

        pos += entry->size;
    }

    (void)fclose(fp);

    if((r = vsc_fopen(dir_name, "wb", &fp)) < 0) {
        vsc_fperror(stderr, r, "Unable to open file '%s'", dir_name);
        goto done;
    }

    if((r = croc_dir_write(fp, directory, file_count, 0)) < 0) {
        vsc_fperror(stderr, r, "Error writing directory: %s", dir_name);
        goto done;
    }

    ret = 0;
done:
    if(directory != NULL)
        vsc_free(directory);

    if(fp != NULL)
        fclose(fp);

    return ret;
}
