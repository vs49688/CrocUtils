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
#include <assert.h>
#include <vsclib.h>
#include <libcroc/wad.h>
#include <libcroc/util.h>


/* Is the file a Saturn MPLOAD%02u.WAD? */
int croc_wad_is_mpload(const char *path, unsigned int *level)
{
	unsigned int _level;
	int pos;
	char c;
	const char *filename = croc_util_get_filename(path);

	if(sscanf(filename, "%*1[mM]%*1[pP]%*1[lL]%*1[oO]%*1[aA]%*1[dD]%2u.%*1[wW]%*1[aA]%c%n", &_level, &c, &pos) != 2)
		return 0;

	/* sscanf doesn't like that last "%1[dD] for some reason. */
	if(c != 'd' && c != 'D')
		return 0;

	if(filename[pos] != '\0')
		return 0;

	if(level != NULL)
		*level = _level;

	return 1;
}

/* Is the file a PSX MAP%02u.WAD? */
int croc_wad_is_mapxx(const char *path, unsigned int *level)
{
	unsigned int _level;
	int pos;
	char c;
	const char *filename = croc_util_get_filename(path);

	if(sscanf(filename, "%*1[mM]%*1[aA]%*1[pP]%2u.%*1[wW]%*1[aA]%c%n", &_level, &c, &pos) != 2)
		return 0;

	/* sscanf doesn't like that last "%1[dD] for some reason. */
	if(c != 'd' && c != 'D')
		return 0;

	if(filename[pos] != '\0')
		return 0;

	if(level != NULL)
		*level = _level;

	return 1;
}

typedef struct IdxParseState
{
    size_t entry_index;
    size_t item_index;
    CrocWadEntry *entries;
} IdxParseState;


static int croc_wad_parse_item(const char *s, const char *e, void *user)
{
    IdxParseState *idx = user;
    CrocWadEntry *ent = idx->entries + idx->entry_index;
    char *end = (char*)s;

    switch(idx->item_index) {
        case 0: /* Filename */
            if(!(ent->filename = malloc(e - s + 1))) {
                errno = ENOMEM;
                return -1;
            }

            memcpy(ent->filename, s, e - s);
            ent->filename[e - s] = '\0';
            break;

        case 1: /* File offset. */
            errno = 0;
            ent->offset = (uint32_t)strtoul(s, &end, 10);
            if(errno || end != e)
                goto fail;
            break;

        case 2: /* Compressed size. */
            errno = 0;
            ent->compressed_size = (uint32_t)strtoul(s, &end, 10);
            if(errno || end != e)
                goto fail;
            break;

        case 3: /* Uncompressed size. */
            errno = 0;
            ent->uncompressed_size = (uint32_t)strtoul(s, &end, 10);
            if(errno || end != e)
                goto fail;
            break;

        case 4: /* Compression type. */
            if(e != s + 1)
                goto fail;

            switch(*s) {
                case 'b': ent->rle_type = CROC_WAD_RLE_BYTE; break;
                case 'w': ent->rle_type = CROC_WAD_RLE_WORD; break;
                case 'u': ent->rle_type = CROC_WAD_RLE_NONE; break;
                default:
                    goto fail;
            }
            break;

        default:
            goto fail;
    }

    idx->item_index = (idx->item_index + 1) % 5;
    return 0;

fail:
    errno = EINVAL;
    return -1;
}

static int croc_wad_parse_line(const char *s, const char *e, void *user)
{
    int r;
    IdxParseState *idx = user;

    /* Handle trailing CRs */
    if(e > s && *(e - 1) == '\r')
        --e;

    /* Skip empty lines. */
    if(s == e)
        return 0;

    if((r = vsc_for_each_delim(s, e, ',', croc_wad_parse_item, idx)) < 0)
        return r;

    ++idx->entry_index;
    return 0;
}

CrocWadEntry *croc_wad_read_index(FILE *f, size_t *num)
{
    void *data = NULL;
    size_t size = 0, num_ = 0;
    const char *start;
    CrocWadEntry *entries = NULL;
    IdxParseState idx;

    /* We need to do two passes, so just load the whole thing. */
    if(vsc_freadall(&data, &size, f) < 0)
        return NULL;

    /* Count the rough number of entries. */
    start = data;
    for(size_t i = 0; i < size; ++i) {
        if(start[i] == '\n')
            ++num_;
    }

    if((entries = calloc(num_, sizeof(CrocWadEntry))) == NULL) {
        errno = ENOMEM;
        goto fail;
    }

    idx.item_index  = 0;
    idx.entry_index = 0;
    idx.entries     = entries;
    if(vsc_for_each_delim(start, start + size, '\n', croc_wad_parse_line, &idx) < 0)
        goto fail;

    free(data);
    *num = idx.entry_index;
    return entries;

fail:
    if(entries)
        croc_wad_free_index(entries, idx.entry_index);

    if(data)
        free(data);

    return NULL;
}

void croc_wad_free_index(CrocWadEntry *entries, size_t num)
{
    if(entries == NULL)
        return;

    for(size_t i = 0; i < num; ++i) {
        if(entries[i].filename)
            free(entries[i].filename);
    }

    free(entries);
}

cJSON *croc_wad_index_write_json(const CrocWadEntry *entries, size_t count)
{
    cJSON *jarr, *jent;
    const char *rle;

    if((jarr = cJSON_CreateArray()) == NULL)
        return NULL;

    for(size_t i = 0; i < count; ++i) {
        const CrocWadEntry *e = entries + i;

        if((jent = cJSON_CreateObject()) == NULL)
            goto fail;

        if(!cJSON_AddItemToArray(jarr, jent))
            goto fail;

        if(cJSON_AddStringToObject(jent, "name", e->filename) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "offset", e->offset) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "compressed_size", e->compressed_size) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "uncompressed_size", e->uncompressed_size) == NULL)
            goto fail;

        switch(e->rle_type) {
            case CROC_WAD_RLE_BYTE: rle = "byte"; break;
            case CROC_WAD_RLE_WORD: rle = "word"; break;
            case CROC_WAD_RLE_NONE: rle = "none"; break;
            default: assert(0);
        }
        if(cJSON_AddStringToObject(jent, "rle_type", rle) == NULL)
            goto fail;
    }

    return jarr;

fail:
    cJSON_Delete(jarr);
    return NULL;
}


void *croc_wad_load_entry(FILE *wad, const CrocWadEntry *entry)
{
    void *cbuf = NULL, *ubuf = NULL;
    int errno_, r;
    size_t bufsize;

    if(!(cbuf = malloc(entry->compressed_size))) {
        errno = ENOMEM;
        goto fail;
    }

    /*
     * The output buffer size should always be a multiple of the RLE size.
     * Round up if need-be. There's only two sizes, so this is fine.
     */
    bufsize = entry->uncompressed_size;
    if(bufsize & 1)
        ++bufsize;

    if(entry->rle_type != CROC_WAD_RLE_NONE && !(ubuf = malloc(bufsize))) {
        errno = ENOMEM;
        goto fail;
    }

    if(vsc_fseeko(wad, entry->offset, SEEK_SET) < 0)
        goto fail;

    if(fread(cbuf, entry->compressed_size, 1, wad) != 1) {
        errno = EIO;
        goto fail;
    }

    if(entry->rle_type == CROC_WAD_RLE_NONE) {
        free(ubuf);
        ubuf = cbuf;
        cbuf = NULL;
        r = 0;
    } else if(entry->rle_type == CROC_WAD_RLE_BYTE) {
        r = croc_wad_decompressb(ubuf, cbuf, entry->compressed_size, bufsize);
    } else if(entry->rle_type == CROC_WAD_RLE_WORD) {
        r = croc_wad_decompressw(ubuf, cbuf, entry->compressed_size, bufsize);
    } else {
        assert(0);
        errno = EOVERFLOW;
        r = -1;
    }

    if(r < 0)
        return NULL;

    if(cbuf)
        free(cbuf);

    return ubuf;

fail:
    errno_ = errno;
    if(ubuf)
        free(ubuf);

    if(cbuf)
        free(cbuf);

    errno = errno_;

    return NULL;
}
