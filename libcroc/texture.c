/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <errno.h>
#include <vsclib.h>
#include <libcroc/col.h>
#include <libcroc/chunk.h>
#include <libcroc/tex.h>

enum {
    TEX_STATE_HEADER            = 0,
    TEX_STATE_PIXELMAP_INFO     = 1,
    TEX_STATE_PIXELMAP_PAL      = 2,
    TEX_STATE_PIXELMAP_PAL_DATA = 3,
    TEX_STATE_PIXELMAP_PAL_GLUE = 4,
    TEX_STATE_PIXELMAP_DATA     = 5,
    TEX_STATE_DONE,
};

typedef struct TexParseState {
    CrocTexture *tex;

    int state; /* One of the TEX_STATE_* constants */
    int pixno;
} TexParseState;


static const uint16_t texsizes[] = {
    [CROC_TEXFMT_INDEX8]       = 1,
    [CROC_TEXFMT_XRGB1555]     = 2,
    [CROC_TEXFMT_RGB565]       = 2,
    [CROC_TEXFMT_RGB888]       = 3,
    [CROC_TEXFMT_XRGB8888]     = 4,
    [CROC_TEXFMT_RGBA8888]     = 4,
    [CROC_TEXFMT_RGBA8888_ARR] = 4,
};

static int is_known_format(CrocTextureFormat fmt)
{
    switch(fmt) {
        case CROC_TEXFMT_INDEX8:
        case CROC_TEXFMT_XRGB1555:
        case CROC_TEXFMT_RGB565:
        case CROC_TEXFMT_RGB888:
        case CROC_TEXFMT_XRGB8888:
        case CROC_TEXFMT_RGBA8888:
        case CROC_TEXFMT_RGBA8888_ARR:
            return 1;
        default:
            fprintf(stderr, "Found texture with unknown format %u, please send this\n", (uint32_t)fmt);
            fprintf(stderr, "  to the developers.\n");
            return 0;
    }
}


static int parse_pixelmap(CrocTexture *tex, CrocChunkType type, const uint8_t *ptr, size_t size)
{
    const uint8_t *end = ptr + size;

    if(type != CROC_CHUNK_PIXELMAP && type != CROC_CHUNK_TYPE_PIXELMAP2)
        return -1;

    if((type == CROC_CHUNK_PIXELMAP && size < 11) || (type == CROC_CHUNK_TYPE_PIXELMAP2 && size < 13))
        return -1;

    tex->format         = vsc_read_uint8(ptr +  0);
    tex->bytes_per_row  = vsc_read_beu16(ptr +  1);
    tex->width          = vsc_read_beu16(ptr +  3);
    tex->height         = vsc_read_beu16(ptr +  5);
    tex->xorigin        = vsc_read_be16( ptr +  7);
    tex->yorigin        = vsc_read_be16( ptr +  9);

    /* TODO: Handle more formats. */
    if(!is_known_format(tex->format))
        return -1;

    /*
     * PIXELMAP2 has an extra 16-bit field at offset 11. Just skip it.
     */
    if(type == CROC_CHUNK_PIXELMAP)
        ptr += 11;
    else
        ptr += 13;

    if((tex->name = vsc_strdupr((const char *)ptr, (const char*)end)) == NULL) {
        errno = ENOMEM;
        return -1;
    }

    return 0;
}

static int read_pixeldata(CrocTexture *tex, CrocChunkType type, const uint8_t *ptr, size_t size)
{
    uint32_t length_, elemsize_;

    if(type != CROC_CHUNK_TYPE_PIXELDATA)
        return -1;

    if(size != (tex->bytes_per_row * tex->height) + 8)
        return -1;

    length_   = vsc_read_beu32(ptr + 0);
    elemsize_ = vsc_read_beu32(ptr + 4);

    if(elemsize_ != texsizes[tex->format])
        return -1;

    if(length_ != (size - 8) / elemsize_)
        return -1;

    if((tex->data = vsc_malloc(size - 8)) == NULL)
        return -1;

    memcpy(tex->data, ptr + 8, size - 8);

    /* Handle pixel endianness. */
    switch(elemsize_) {
        case 1:
            break;

        case 2: {
            uint16_t *data = tex->data;
            for(size_t i = 0; i < length_; ++i)
                data[i] = vsc_beu16_to_native(data[i]);
            break;
        }

        case 3: {
            uint8_t *data = tex->data;
            for(size_t i = 0; i < size - 8; i += 3) {
                uint8_t c   = data[i + 0];
                data[i + 0] = data[i + 2];
                data[i + 2] = c;
            }
            break;
        }

        case 4: {
            uint32_t *data = tex->data;
            for(size_t i = 0; i < length_; ++i)
                data[i] = vsc_beu32_to_native(data[i]);
            break;
        }

        default:
            assert(0);

    }
    return 0;
}

static int enumproc(CrocChunkType type, const uint8_t *ptr, size_t size, void *user)
{
    int r;
    TexParseState *state = user;

    if(state->pixno > 0 && state->state == TEX_STATE_HEADER && type != CROC_CHUNK_TYPE_HEADER)
        state->state = TEX_STATE_PIXELMAP_INFO;

    if(state->state == TEX_STATE_HEADER) {
        uint32_t type_, version_;

        if(state->pixno > 0 || type != CROC_CHUNK_TYPE_HEADER || size != 8)
            return -1;

        type_    = vsc_read_beu32(ptr + 0);
        version_ = vsc_read_beu32(ptr + 4);

        if(type_ != CROC_FILE_TYPE_PIXELMAP || version_ != 2)
            return -1;

        state->state = TEX_STATE_PIXELMAP_INFO;
        return 0;
    }

    if(state->state == TEX_STATE_PIXELMAP_INFO) {
        if((r = parse_pixelmap(state->tex, type, ptr, size)) < 0)
            return r;

        if(state->tex->format == CROC_TEXFMT_INDEX8)
            state->state = TEX_STATE_PIXELMAP_PAL;
        else
            state->state = TEX_STATE_PIXELMAP_DATA;

        return 0;
    }

    if(state->state == TEX_STATE_PIXELMAP_PAL) {
        assert(state->tex->format == CROC_TEXFMT_INDEX8);

        /* Some files have external palettes. Handle this case. */
        if(type != CROC_CHUNK_TYPE_PIXELDATA) {
            if((state->tex->palette = vsc_calloc(1, sizeof(CrocTexture))) == NULL)
                return -1;

            if((r = parse_pixelmap(state->tex->palette, type, ptr, size)) < 0)
                return r;

            state->state = TEX_STATE_PIXELMAP_PAL_DATA;
            return 0;
        }

        state->state = TEX_STATE_PIXELMAP_DATA;
    }

    if(state->state == TEX_STATE_PIXELMAP_PAL_DATA) {
        if((r = read_pixeldata(state->tex->palette, type, ptr, size)) < 0)
            return r;

        state->state = TEX_STATE_PIXELMAP_PAL_GLUE;
        return 0;
    }

    if(state->state == TEX_STATE_PIXELMAP_PAL_GLUE) {
        if(type != CROC_CHUNK_TYPE_PAL_GLUE)
            return -1;

        state->state = TEX_STATE_PIXELMAP_DATA;
        return 0;
    }

    if(state->state == TEX_STATE_PIXELMAP_DATA) {
        if((r = read_pixeldata(state->tex, type, ptr, size)) < 0)
            return r;

        state->state = TEX_STATE_DONE;
        ++state->pixno;
        return 0;
    }

    return -1;
}


int croc_texture_read_many(FILE *f, CrocTexture **textures, size_t *num)
{
    int errno_, i;

    TexParseState state = {
        .tex   = NULL,
        .state = TEX_STATE_HEADER,
        .pixno = 0,
    };

    state.tex   = NULL;
    state.state = TEX_STATE_HEADER;

    for(i = 0; i < CROC_TEXTURE_MAX_COUNT; ++i) {
        if((textures[i] = vsc_calloc(1, sizeof(CrocTexture))) == NULL) {
            errno = ENOMEM;
            goto fail;
        }

        state.tex     = textures[i];
        state.state   = TEX_STATE_HEADER;

        if(croc_chunk_enumerate(f, enumproc, &state) >= 0)
            continue;

        if(errno != EIO)
            goto fail;

        /* EIO needs special handling. */
        if(ferror(f) || !feof(f) || state.state != TEX_STATE_HEADER)
            goto fail;

        vsc_free(textures[i]);
        textures[i] = NULL;
        break;
    }

    *num = (size_t)i;
    return 0;
fail:
    errno_ = errno;

    croc_texture_free_many(textures, (size_t)i + 1);

    errno = errno_;
    return -1;
}

void croc_texture_free_many(CrocTexture **textures, size_t num)
{
    if(textures == NULL || num == 0)
        return;

    for(size_t i = num; i-- > 0;) {
        if(textures[i]->palette)
            croc_texture_free(textures[i]->palette);

        croc_texture_free(textures[i]);
        textures[i] = NULL;
    }
}

void croc_texture_free(CrocTexture *tex)
{
    if(tex == NULL)
        return;

    if(tex->data)
        vsc_free(tex->data);

    if(tex->name)
        vsc_free(tex->name);

    vsc_free(tex);
}

CrocTexture *croc_texture_allocate(uint16_t width, uint16_t height, CrocTextureFormat format)
{
    CrocTexture *tex = vsc_calloc(1, sizeof(CrocTexture));

    if(tex == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    tex->format        = format;
    tex->width         = width;
    tex->height        = height;
    tex->bytes_per_row = width * texsizes[format];

    if((tex->data = vsc_calloc(tex->height, tex->bytes_per_row)) == NULL) {
        vsc_free(tex);
        errno = ENOMEM;
        return NULL;
    }

    return tex;
}

CrocTexture *croc_texture_rgb565_to_rgba8888(const CrocTexture *texture, const CrocColour *key)
{
    CrocTexture *tex;
    const uint16_t *in;
    uint32_t *out;

    if(texture == NULL || texture->data == NULL || texture->format != CROC_TEXFMT_RGB565) {
        errno = EINVAL;
        return NULL;
    }

    tex = croc_texture_allocate(texture->width, texture->height, CROC_TEXFMT_RGBA8888);
    if(tex == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if(texture->name) {
        if((tex->name = vsc_strdup(texture->name)) == NULL) {
            croc_texture_free(tex);
            errno = ENOMEM;
            return NULL;
        }
    }

    in  = texture->data;
    out = tex->data;
    for(size_t i = 0; i < tex->width * tex->height; ++i) {
        CrocColour col = croc_colour_unpack_rgb565(in[i]);
        if(key != NULL && col.r == key->r && col.g == key->g && col.b == key->b)
            col.pad = 0x00u;

        out[i] = croc_colour_pack_rgba8888(col);
    }

    return tex;
}

CrocTexture *croc_texture_rgb888_to_rgba8888(const CrocTexture *tex, const CrocColour *key)
{
    const uint8_t *in;
    uint32_t *out;
    CrocTexture *ntex;

    if(tex == NULL || tex->format != CROC_TEXFMT_RGB888) {
        errno = EINVAL;
        return NULL;
    }

    if((ntex = croc_texture_allocate(tex->width, tex->height, CROC_TEXFMT_RGBA8888)) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if(tex->name) {
        if((ntex->name = vsc_strdup(tex->name)) == NULL) {
            croc_texture_free(ntex);
            errno = ENOMEM;
            return NULL;
        }
    }

    in = tex->data;
    out = ntex->data;
    for(size_t i = 0; i < tex->width * tex->height; ++i, in += 3) {
        CrocColour col = croc_colour_unpack_rgb888(in);
        if(key != NULL && col.r == key->r && col.g == key->g && col.b == key->b)
            col.pad = 0x00u;

        out[i] = croc_colour_pack_rgba8888(col);
    }

    return ntex;
}

int croc_texture_xrgb1555_to_rgb565(CrocTexture *tex)
{
    uint16_t *data;

    if(tex == NULL || tex->format != CROC_TEXFMT_XRGB1555) {
        errno = EINVAL;
        return -1;
    }

    data = tex->data;
    for(size_t i = 0; i < tex->width * tex->height; ++i)
        data[i] = croc_colour_pack_rgb565(croc_colour_unpack_xrgb1555(data[i]));

    tex->format = CROC_TEXFMT_RGB565;
    return 0;
}

CrocTexture *croc_texture_deindex8(const CrocTexture *tex, const CrocColour *key)
{
    uint8_t *index;
    uint32_t *data, *pal;
    CrocTexture *ntex;

    if(tex == NULL || tex->format != CROC_TEXFMT_INDEX8) {
        errno = EINVAL;
        return NULL;
    }

    /* I've only ever seen indexed textures using XRGB8888 */
    if(tex->palette == NULL || tex->palette->format != CROC_TEXFMT_XRGB8888) {
        errno = EINVAL;
        return NULL;
    }

    if((ntex = croc_texture_allocate(tex->width, tex->height, CROC_TEXFMT_RGBA8888)) == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    if(tex->name) {
        if((ntex->name = vsc_strdup(tex->name)) == NULL) {
            croc_texture_free(ntex);
            errno = ENOMEM;
            return NULL;
        }
    }

    index = tex->data;
    data  = ntex->data;
    pal   = tex->palette->data;
    for(size_t i = 0; i < tex->width * tex->height; ++i) {
        CrocColour col = croc_colour_unpack_xrgb8888(pal[index[i]]);
        if(key != NULL && col.r == key->r && col.g == key->g && col.b == key->b)
            col.pad = 0x00u;

        data[i] = croc_colour_pack_rgba8888(col);
    }

    return ntex;
}
