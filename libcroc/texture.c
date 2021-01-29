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
#include <assert.h>
#include <errno.h>
#include <vsclib.h>
#include <libcroc/col.h>
#include <libcroc/chunk.h>
#include <libcroc/tex.h>

enum {
    TEX_STATE_NONE          = 0,
    TEX_STATE_FILE_INFO     = 1,
    TEX_STATE_PIXELMAP_INFO = 2,
    TEX_STATE_PIXELMAP_DATA = 3,
};

typedef struct TexParseState {
    CrocTexture *tex;
    int state; /* One of the TEX_STATE_* constants */
    int pixno;
} TexParseState;


static const uint16_t texsizes[] = {
    [CROC_TEXFMT_XRGB1555]     = 2,
    [CROC_TEXFMT_RGB565]       = 2,
    [CROC_TEXFMT_RGBA8888]     = 4,
    [CROC_TEXFMT_RGBA8888_ARR] = 4,
};

static int is_known_format(CrocTextureFormat fmt)
{
    switch(fmt) {
        case CROC_TEXFMT_XRGB1555:
        case CROC_TEXFMT_RGB565:
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

    if((tex->name = vsc_strrdup((const char *)ptr, (const char*)end)) == NULL)
        return -1;

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

    if((tex->data = malloc(size - 8)) == NULL)
        return -1;

    memcpy(tex->data, ptr + 8, size - 8);

    /* Handle pixel endianness. */
    switch(elemsize_) {
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

    if(state->pixno > 0 && state->state == TEX_STATE_NONE && type != CROC_CHUNK_TYPE_HEADER)
        state->state = TEX_STATE_FILE_INFO;

    if(state->state == TEX_STATE_NONE) {
        uint32_t type_, version_;

        if(state->pixno > 0 || type != CROC_CHUNK_TYPE_HEADER || size != 8)
            return -1;

        type_    = vsc_read_beu32(ptr + 0);
        version_ = vsc_read_beu32(ptr + 4);

        if(type_ != CROC_FILE_TYPE_PIXELMAP || version_ != 2)
            return -1;

        state->state = TEX_STATE_FILE_INFO;
        return 0;
    }

    if(state->state == TEX_STATE_FILE_INFO) {
        if((r = parse_pixelmap(state->tex, type, ptr, size)) < 0)
            return r;

        state->state = TEX_STATE_PIXELMAP_INFO;
        return 0;
    }

    if(state->state == TEX_STATE_PIXELMAP_INFO) {
        if((r = read_pixeldata(state->tex, type, ptr, size)) < 0)
            return r;

        state->state = TEX_STATE_PIXELMAP_DATA;
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
        .state = TEX_STATE_NONE,
        .pixno = 0,
    };

    state.tex   = NULL;
    state.state = TEX_STATE_NONE;

    for(i = 0; i < CROC_TEXTURE_MAX_COUNT; ++i) {
        if((textures[i] = calloc(1, sizeof(CrocTexture))) == NULL) {
            errno = ENOMEM;
            goto fail;
        }

        state.tex     = textures[i];
        state.state   = TEX_STATE_NONE;

        if(croc_chunk_enumerate(f, enumproc, &state) >= 0)
            continue;

        if(errno != EIO)
            goto fail;

        /* EIO needs special handling. */
        if(ferror(f) || !feof(f) || state.state != TEX_STATE_NONE)
            goto fail;

        free(textures[i]);
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
        croc_texture_free(textures[i]);
        textures[i] = NULL;
    }
}

void croc_texture_free(CrocTexture *tex)
{
    if(tex == NULL)
        return;

    if(tex->data)
        free(tex->data);

    if(tex->name)
        free(tex->name);

    free(tex);
}

CrocTexture *croc_texture_allocate(uint16_t width, uint16_t height, CrocTextureFormat format)
{
    CrocTexture *tex = calloc(1, sizeof(CrocTexture));

    if(tex == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    tex->format        = format;
    tex->width         = width;
    tex->height        = height;
    tex->bytes_per_row = width * texsizes[format];

    if((tex->data = calloc(tex->height, tex->bytes_per_row)) == NULL) {
        free(tex);
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
        if((tex->name = strdup(texture->name)) == NULL) {
            free(tex);
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
