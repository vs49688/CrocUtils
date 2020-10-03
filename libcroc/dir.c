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
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <vsclib.h>
#include <cJSON.h>
#include <libcroc/dir.h>

static int croc_dir_read_entry(CrocDirEntry *d, FILE *f, int big, int oldstyle)
{
    uint8_t buf[CROC_DIRENTRY_SIZE];
    uint32_t sr;

    if(fread(buf, CROC_DIRENTRY_SIZE, 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    memcpy(d->name, buf, CROC_DIRENTRY_NAME_SIZE);
    d->name[CROC_DIRENTRY_NAME_SIZE] = '\0';
    if(big) {
        sr              = vsc_read_beu32(buf + 12);
        d->offset       = vsc_read_beu32(buf + 16);
        d->usage        = vsc_read_beu32(buf + 20);
    } else {
        sr              = vsc_read_leu32(buf + 12);
        d->offset       = vsc_read_leu32(buf + 16);
        d->usage        = vsc_read_leu32(buf + 20);
    }

    d->size    = sr & 0x00FFFFFF;
    d->restart = (uint8_t)((sr >> 24) & 0xFF);

    /*
     * Old-style directories have the sector.
     * Convert it to an offset.
     */
    if(oldstyle)
        d->offset *= CROC_DIRENTRY_SECTOR_SIZE;

    return 0;
}

CrocDirEntry *croc_dir_read(FILE *f, size_t *count, int *old)
{
    uint32_t _count, start = 0;
    int err = 0, big = 0, oldstyle = 0;
    CrocDirEntry *entries = NULL;
    CrocDirEntry tmp[2];

    if(f == NULL) {
        errno = EINVAL;
        return NULL;
    }

    _count = vsc_fread_leu32(f);

    if(feof(f) || ferror(f)) {
        errno = EIO;
        goto done;
    }

    /* See if we're big endian. */
    if(_count > 0x0000FFFFu) {
        _count = vsc_swap_uint32(_count);
        big = 1;
    }

    /* Read the first two to see if we're an old-style index. */
    if(_count >= 2) {
        if(croc_dir_read_entry(tmp + 0, f, big, 0) < 0)
            goto done;

        if(croc_dir_read_entry(tmp + 1, f, big, 0) < 0)
            goto done;

        if((oldstyle = tmp[1].offset < tmp[0].size)) {
            _count /= CROC_DIRENTRY_SIZE;

            tmp[0].offset *= CROC_DIRENTRY_SECTOR_SIZE;
            tmp[1].offset *= CROC_DIRENTRY_SECTOR_SIZE;
        }
        start = 2;
    }

    if((entries = calloc(_count, sizeof(CrocDirEntry))) == NULL)
        goto done;

    if(_count >= 2) {
        entries[0] = tmp[0];
        entries[1] = tmp[1];
    }

    for(uint32_t i = start; i < _count; ++i) {
        if(croc_dir_read_entry(entries + i, f, big, oldstyle) < 0)
            goto done;
    }

    if(old != NULL)
        *old = oldstyle;

    *count = _count;
    return entries;
done:
    err = errno;

    if(entries != NULL)
        free(entries);

    errno = err;
    return NULL;
}

cJSON *croc_dir_write_json(const CrocDirEntry *entries, size_t count)
{
    cJSON *jarr, *jent;

    if((jarr = cJSON_CreateArray()) == NULL)
        return NULL;

    for(size_t i = 0; i < count; ++i) {
        const CrocDirEntry *e = entries + i;

        if((jent = cJSON_CreateObject()) == NULL)
            goto fail;

        if(!cJSON_AddItemToArray(jarr, jent))
            goto fail;

        if(cJSON_AddStringToObject(jent, "name", e->name) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "size", e->size) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "restart", e->restart) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "offset", e->offset) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "usage", e->usage) == NULL)
            goto fail;
    }

    return jarr;

fail:
    cJSON_Delete(jarr);
    return NULL;
}
