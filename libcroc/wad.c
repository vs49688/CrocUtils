/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <vsclib.h>
#include <libcroc/wad.h>
#include <libcroc/util.h>

const char *const CrocWadPaths[] = {
    [CROC_WAD_NONE]              = "",
    [CROC_WAD_ANIMS]             = "anims",
    [CROC_WAD_MAPS]              = "maps",
    [CROC_WAD_OBJECTS]           = "objects",
    [CROC_WAD_SOUND]             = "sound",
    [CROC_WAD_TEXTURES]          = "textures",
    [CROC_WAD_FONTS]             = "fonts",
    [CROC_WAD_MATERIAL]          = "material",
    [CROC_WAD_MATERIAL_ICE]      = "material/ice",
    [CROC_WAD_MATERIAL_DESERT]   = "material/desert",
    [CROC_WAD_MATERIAL_WATER]    = "material/water",
    [CROC_WAD_MATERIAL_CASTLE]   = "material/castle",
    [CROC_WAD_MATERIAL_CAVE]     = "material/cave",
    [CROC_WAD_MATERIAL_FOREST]   = "material/forest",
    [CROC_WAD_MATERIAL_ICECAVE]  = "material/icecave",
    [CROC_WAD_MATERIAL_DUNGEON]  = "material/dungeon",
    [CROC_WAD_MATERIAL_ISLAND1]  = "material/island1",
    [CROC_WAD_MATERIAL_ISLAND2]  = "material/island2",
    [CROC_WAD_MATERIAL_ISLAND3]  = "material/island3",
    [CROC_WAD_MATERIAL_ISLAND4]  = "material/island4",
    [CROC_WAD_MATERIAL_ISLAND5]  = "material/island5",
    [CROC_WAD_MATERIAL_BOSS1]    = "material/boss1",
    [CROC_WAD_MATERIAL_BOSS2]    = "material/boss2",
    [CROC_WAD_MATERIAL_BOSS3]    = "material/boss3",
    [CROC_WAD_MATERIAL_BOSS4]    = "material/boss4",
    [CROC_WAD_MATERIAL_BOSS5]    = "material/boss5",
    [CROC_WAD_MATERIAL_BOSS6]    = "material/boss6",
    [CROC_WAD_MATERIAL_BOSS7]    = "material/boss7",
    [CROC_WAD_MATERIAL_BOSS8]    = "material/boss8",
    [CROC_WAD_MATERIAL_BOSS9]    = "material/boss9",
    [CROC_WAD_MATERIAL_INTRO]    = "material/intro",
    [CROC_WAD_MATERIAL_INTROJIG] = "material/intro",
    [CROC_WAD_MATERIAL_OUTRO]    = "material/outro",
    [CROC_WAD_PALETTES]          = "palettes",
    [CROC_WAD_STRATS]            = "strats",
};

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
    int r;

    switch(idx->item_index) {
        case 0: /* Filename */
            if(!(ent->filename = vsc_malloc(e - s + 1)))
                return VSC_ERROR(ENOMEM);

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
    return VSC_ERROR(EINVAL);
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

int croc_wad_read_index(FILE *f, CrocWadEntry **_entries, size_t *num)
{
    void *data = NULL;
    size_t size = 0, num_ = 0;
    const char *start;
    CrocWadEntry *entries = NULL;
    IdxParseState idx;
    int r;

    /* We need to do two passes, so just load the whole thing. */
    if((r = vsc_freadall(&data, &size, f)) < 0)
        return r;

    /* Count the rough number of entries. */
    start = data;
    for(size_t i = 0; i < size; ++i) {
        if(start[i] == '\n')
            ++num_;
    }

    if((entries = vsc_calloc(num_, sizeof(CrocWadEntry))) == NULL) {
        r = VSC_ERROR(ENOMEM);
        goto fail;
    }

    idx.item_index  = 0;
    idx.entry_index = 0;
    idx.entries     = entries;
    if((r = vsc_for_each_delim(start, start + size, '\n', croc_wad_parse_line, &idx)) < 0)
        goto fail;

    vsc_free(data);
    *_entries = entries;
    *num = idx.entry_index;
    return 0;

fail:
    if(entries)
        croc_wad_free_index(entries, idx.entry_index);

    if(data)
        vsc_free(data);

    return r;
}

void croc_wad_free_index(CrocWadEntry *entries, size_t num)
{
    if(entries == NULL)
        return;

    for(size_t i = 0; i < num; ++i) {
        if(entries[i].filename)
            vsc_free(entries[i].filename);
    }

    vsc_free(entries);
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


int croc_wad_load_entry(FILE *wad, const CrocWadEntry *entry, void **buf)
{
    void *cbuf = NULL, *ubuf = NULL;
    int r;
    size_t bufsize;

    if(wad == NULL || entry == NULL || buf == NULL)
        return VSC_ERROR(EINVAL);

    if((cbuf = vsc_malloc(entry->compressed_size)) == NULL)
        return VSC_ERROR(ENOMEM);

    /*
     * The output buffer size should always be a multiple of the RLE size.
     * Round up if need-be. There's only two sizes, so this is fine.
     */
    bufsize = entry->uncompressed_size;
    if(bufsize & 1)
        ++bufsize;

    if(entry->rle_type != CROC_WAD_RLE_NONE && !(ubuf = vsc_malloc(bufsize))) {
        r = VSC_ERROR(ENOMEM);
        goto fail;
    }

    if((r = vsc_fseeko(wad, entry->offset, SEEK_SET)) < 0)
        goto fail;

    if(fread(cbuf, entry->compressed_size, 1, wad) != 1) {
        r = VSC_ERROR(EIO);
        goto fail;
    }

    if(entry->rle_type == CROC_WAD_RLE_NONE) {
        vsc_free(ubuf);
        ubuf = cbuf;
        cbuf = NULL;
        r = 0;
    } else if(entry->rle_type == CROC_WAD_RLE_BYTE) {
        r = croc_wad_decompressb(ubuf, cbuf, entry->compressed_size, bufsize);
    } else if(entry->rle_type == CROC_WAD_RLE_WORD) {
        r = croc_wad_decompressw(ubuf, cbuf, entry->compressed_size, bufsize);
    } else {
        r = VSC_ERROR(EOVERFLOW);
    }

    if(r < 0)
        return r;

    if(cbuf)
        vsc_free(cbuf);

    *buf = ubuf;
    return 0;

fail:
    if(ubuf)
        vsc_free(ubuf);

    if(cbuf)
        vsc_free(cbuf);

    return r;
}

/* FIXME: This should be in vsclib */
#if defined(_MSC_VER)
#   define strcasecmp stricmp
#endif

static int index_sort(const void *a, const void *b)
{
    const CrocWadEntry *ia = a;
    const CrocWadEntry *ib = b;
    return strcasecmp(ia->filename, ib->filename);
}

static int index_compare(const void *key, const void *elem)
{
    const char         *name  = key;
    const CrocWadEntry *entry = elem;
    return strcasecmp(name, entry->filename);
}

int croc_wadfs_open(CrocWadFs **fs, const char *base)
{
    CrocWadFs *wadfs = NULL;
    FILE      *fp    = NULL;
    int        r     = VSC_ERROR(EINVAL);

    if((wadfs = vsc_calloc(1, sizeof(CrocWadFs))) == NULL)
        return VSC_ERROR(ENOMEM);

    if((wadfs->idx_path = vsc_asprintf("%s.idx", base)) == NULL) {
        r = VSC_ERROR(ENOMEM);
        goto fail;
    }

    if((wadfs->wad_path = vsc_asprintf("%s.wad", base)) == NULL) {
        r = VSC_ERROR(ENOMEM);
        goto fail;
    }

    if((r = vsc_fopen(wadfs->idx_path, "rb", &fp)) < 0)
        goto fail;

    if((r = croc_wad_read_index(fp, &wadfs->entries, &wadfs->num_entries)) < 0)
        goto fail;

    /* Sort them for quick bsearch()'ing. */
    qsort(wadfs->entries, wadfs->num_entries, sizeof(CrocWadEntry), index_sort);

    fclose(fp);

    if((r = vsc_fopen(wadfs->wad_path, "rb", &wadfs->wad)) < 0)
        goto fail;

    *fs = wadfs;
    return 0;

fail:
    if(fp != NULL)
        fclose(fp);

    croc_wadfs_close(wadfs);
    return r;
}

int croc_wadfs_load(CrocWadFs *fs, const char *name, void **data, const CrocWadEntry **const _entry)
{
    CrocWadEntry *entry;
    int r;

    if(name == NULL || data == NULL)
        return VSC_ERROR(EINVAL);

    if((entry = bsearch(name, fs->entries, fs->num_entries, sizeof(CrocWadEntry), index_compare)) == NULL) {
        return VSC_ERROR(ENOENT);
    }

    if((r = croc_wad_load_entry(fs->wad, entry, data)) < 0)
        return r;

    if(_entry != NULL)
        *_entry = entry;

    return 0;
}

void croc_wadfs_close(CrocWadFs *fs)
{
    if(fs == NULL)
        return;

    if(fs->wad != NULL)
        fclose(fs->wad);

    if(fs->entries != NULL)
        vsc_free(fs->entries);

    if(fs->wad_path != NULL)
        vsc_free(fs->wad_path);

    if(fs->idx_path != NULL)
        vsc_free(fs->idx_path);

    vsc_free(fs);
}
