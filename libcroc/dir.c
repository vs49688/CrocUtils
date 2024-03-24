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

static int croc_dir_write_entry(const CrocDirEntry *d, FILE *f, int big)
{
    uint8_t  buf[CROC_DIRENTRY_SIZE];
    uint32_t sr = (d->size & 0xFFFFFF) | (d->restart << 24);

    memcpy(buf + 0, d->name, CROC_DIRENTRY_NAME_SIZE);
    if(big) {
        vsc_write_beu32(buf + 12, sr);
        vsc_write_beu32(buf + 16, d->offset);
        vsc_write_beu32(buf + 20, d->usage);
    } else {
        vsc_write_leu32(buf + 12, sr);
        vsc_write_leu32(buf + 16, d->offset);
        vsc_write_leu32(buf + 20, d->usage);
    }

    if(fwrite(buf, CROC_DIRENTRY_SIZE, 1, f) != 1)
        return VSC_ERROR(EIO);

    return 0;
}

static int croc_dir_read_entry(CrocDirEntry *d, FILE *f, int big, int oldstyle)
{
    uint8_t buf[CROC_DIRENTRY_SIZE];
    uint32_t sr;

    if(fread(buf, CROC_DIRENTRY_SIZE, 1, f) != 1)
        return VSC_ERROR(EIO);

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

int croc_dir_read(FILE *f, CrocDirEntry **entry, size_t *count, int *old)
{
    uint32_t _count, start = 0;
    int ret = 0, big = 0, oldstyle = 0;
    CrocDirEntry *entries = NULL;
    CrocDirEntry tmp[2];

    if(entry == NULL || f == NULL || count == NULL)
        return VSC_ERROR(EINVAL);

    _count = vsc_fread_leu32(f);

    if(feof(f) || ferror(f)) {
        ret = VSC_ERROR(EIO);
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

    if((entries = vsc_calloc(_count, sizeof(CrocDirEntry))) == NULL)
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
    *entry = entries;
    return 0;
done:

    if(entries != NULL)
        vsc_free(entries);

    return ret;
}

int croc_dir_write(FILE *f, const CrocDirEntry *entries, size_t count, int big)
{
    size_t r;
    int ret;

    if(f == NULL || (entries == NULL && count != 0))
        return VSC_ERROR(EINVAL);

    if(count > UINT32_MAX)
        return VSC_ERROR(ERANGE);

    if(big)
        r = vsc_fwrite_beu32(f, (uint32_t)count);
    else
        r = vsc_fwrite_leu32(f, (uint32_t)count);

    if(r != 1)
        return VSC_ERROR(EIO);

    for(size_t i = 0; i < count; ++i) {
        if((ret = croc_dir_write_entry(entries + i, f, big)) < 0)
            return ret;
    }

    return 0;
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
