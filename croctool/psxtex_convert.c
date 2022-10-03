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

#include "stb_image_write.h"
#include <stdio.h>
#include <vsclib.h>
#include <libcroc/psx.h>
#include <libcroc/tex.h>

const static CrocPSXTexture default_texture = {
    ._checksum      = 0,
    .type           = CROC_PSX_TEXTURE_UNCOMPRESSED,
    .num_rects      = 0,
    .num_pages      = 0,
    .num_anims      = 0,
    .rects          = NULL,
    .pages          = NULL,
};

void croc_psx_texture_init(CrocPSXTexture *texture)
{
    *texture = default_texture;
}

static int read_rect(FILE *f, CrocPSXTextureRect *rect, int v4)
{
    uint8_t buf[CROC_PSX_TEXTURE_RECT4_SIZE];
    uint8_t *rectstart;

    _Static_assert(sizeof(buf) >= CROC_PSX_TEXTURE_RECT_SIZE, "sizeof(buf) < CROC_PSX_TEXTURE_RECT_SIZE");

    if(v4) {
        if(fread(buf, CROC_PSX_TEXTURE_RECT4_SIZE, 1, f) != 1)
            return VSC_ERROR(EIO);
        rectstart = buf + 6;
    } else {
        if(fread(buf, CROC_PSX_TEXTURE_RECT_SIZE, 1, f) != 1)
            return VSC_ERROR(EIO);

        rectstart = buf + 4;
    }


    rect->unk_0     = vsc_read_le16(buf + 0);
    rect->unk_2     = vsc_read_le16(buf + 2);
    if(v4)
        rect->unk_2 = vsc_read_le16(buf + 4);

    rect->uv_bl[0] = vsc_read_uint8(rectstart + 0);
    rect->uv_bl[1] = vsc_read_uint8(rectstart + 1);

    rect->uv_br[0] = vsc_read_uint8(rectstart + 2);
    rect->uv_br[1] = vsc_read_uint8(rectstart + 3);

    rect->uv_tl[0] = vsc_read_uint8(rectstart + 4);
    rect->uv_tl[1] = vsc_read_uint8(rectstart + 5);

    rect->uv_tr[0] = vsc_read_uint8(rectstart + 6);
    rect->uv_tr[1] = vsc_read_uint8(rectstart + 7);

    /* Ensure we're square. */
    if(rect->uv_bl[0] != rect->uv_tl[0])
        return VSC_ERROR(EINVAL);

    if(rect->uv_bl[1] != rect->uv_br[1])
        return VSC_ERROR(EINVAL);

    if(rect->uv_br[0] != rect->uv_tr[0])
        return VSC_ERROR(EINVAL);

    if(rect->uv_tl[1] != rect->uv_tr[1])
        return VSC_ERROR(EINVAL);
    return 0;
}

static int read_page(FILE *f, CrocPSXTexturePage *page, CrocPSXTextureType type, int decompress)
{
    uint32_t data_size;
    uint8_t  *buf;

    /* Simple case, uncompressed. */
    if(type == CROC_PSX_TEXTURE_UNCOMPRESSED4 || type == CROC_PSX_TEXTURE_UNCOMPRESSED) {
        if(fread(page->data, CROC_PSX_TEXTURE_TPAGE_SIZE, 1, f) != 1)
            return VSC_ERROR(EIO);

        page->data_size = CROC_PSX_TEXTURE_TPAGE_SIZE;
        return 0;
    }

    /* Are there other types? */
    if(type != CROC_PSX_TEXTURE_RLE16)
        return VSC_ERROR(EINVAL);

    data_size = vsc_fread_leu32(f);

    /* Sanity check. */
    if(data_size > CROC_PSX_TEXTURE_TPAGE_SIZE)
        return VSC_ERROR(ERANGE);

    /* Want compressed data? Sure! */
    if(!decompress) {
        if(fread(page->data, data_size, 1, f) != 1)
            return VSC_ERROR(EIO);

        page->data_size = data_size;
        return 0;
    }

    /* Decompress it. */
    if((buf = vsc_calloc(data_size, 1)) == NULL)
        return VSC_ERROR(ENOMEM);

    if(fread(buf, data_size, 1, f) != 1) {
        vsc_free(buf);
        return VSC_ERROR(EIO);
    }

    if(croc_psx_texture_decompress(page->_data, buf, data_size, sizeof(page->_data)) < 0) {
        vsc_free(buf);
        return VSC_ERROR(errno);
    }

    page->data_size = CROC_PSX_TEXTURE_TPAGE_SIZE;

    return 0;
}

#include <libcroc/col.h>

CrocColour croc_colour_unpack_argb1555(uint16_t pixel)
{
    CrocColour c;
    c.r   = (uint8_t)((pixel & 0x7c00u) >> 10u) << 3u;
    c.g   = (uint8_t)((pixel & 0x03e0u) >> 5u) << 3u;
    c.b   = (uint8_t)((pixel & 0x001fu) >> 0u) << 3u;
    c.pad = (pixel & 0x8000u) ? 0xFF : 0x00;
    return c;
}

CrocColour croc_colour_unpack_rgba5551(uint16_t pixel)
{
    CrocColour c = croc_colour_unpack_xrgb1555(pixel >> 1);
    //c.pad = (pixel & 0x0001u) ? 0xFF : 0x00;
    c.pad = 0xFF;
    uint8_t x = c.r;
    c.r = c.b;
    c.b = x;
    return c;
}

static CrocColour swapredblue(CrocColour col)
{
    uint8_t tmp = col.r;
    col.r = col.b;
    col.b = tmp;
    return col;
}

CrocColour croc_colour_unpack_rgbx5551(uint16_t pixel)
{
    CrocColour c = croc_colour_unpack_xrgb1555((pixel >> 1) & 0x7FFF);


    return c;
}

int croc_psx_tex_burst(const CrocPSXTexture *tex, CrocTexture **textures, size_t *n)
{
    int ret = 0;

    if(tex == NULL || textures == NULL || n == NULL)
        return VSC_ERROR(EINVAL);

    for(size_t i = 0; i < tex->num_rects; ++i) {
        const CrocPSXTextureRect *rect = tex->rects + i;
        CrocTexture              *t;

        t = croc_texture_allocate(
            (rect->uv_tr[0] - rect->uv_tl[0]) * 2 /* 'cause 512 */,
            rect->uv_tr[1] - rect->uv_br[1],
            CROC_TEXFMT_RGBA8888
        );
        if(t == NULL) {
            ret = VSC_ERROR(errno);
            goto fail;
        }

        textures[i] = t;

    }

    for(size_t t = 0; t < tex->num_rects; ++t) {
        const CrocPSXTextureRect *rect    = tex->rects + t;
        const uint8_t            *page    = tex->pages[0].data; // FIXME:
        CrocTexture              *outtex  = textures[t];
        uint32_t                 *outdata = outtex->data;

        size_t src_height        = rect->uv_tr[1] - rect->uv_br[1];
        size_t src_width         = (rect->uv_tr[0] - rect->uv_tl[0]) * 2;
        size_t src_bytes_per_row = 512 * sizeof(uint8_t);
        page += (src_bytes_per_row * rect->uv_tl[1]) + (rect->uv_tl[0] * 2);
        for(size_t j = 0; j < src_height; ++j) {
            for(size_t i = 0; i < src_width; ++i) {
                uint8_t idx = page[i];

                CrocColour col = {
                    .r = idx,
                    .b = idx,
                    .g = idx,
                    .pad = 0xFF,
                };

                *outdata++ = croc_colour_pack_rgba8888(col);
            }

            page += src_bytes_per_row;
        }
    }

    *n = tex->num_rects;
    return 0;

fail:
    for(size_t i = 0; i < tex->num_rects; ++i) {
        if(textures[i] != NULL)
            vsc_free(textures[i]);
    }

    return ret;
}

int croc_psx_tex_to_tex(const CrocPSXTexture *tex, size_t page, CrocTexture **_out)
{
    CrocTexture   *out;
    const uint8_t *indata;
    uint32_t      *outdata;

    if(tex == NULL || _out == NULL)
        return VSC_ERROR(EINVAL);

    if(page >= tex->num_pages)
        return VSC_ERROR(EINVAL);

    if((out = croc_texture_allocate(512, 256, CROC_TEXFMT_RGBA8888)) == NULL)
        return VSC_ERROR(errno);

    indata  = (const uint8_t *)tex->pages[page].data;
    outdata = out->data;


    for(int i = 0; i < 512 * 256; ++i) {
        uint8_t idx = *indata++;

        CrocColour incol = {
            .r = idx,
            .g = idx,
            .b = idx,
            .pad = 0xFF,
        };
//        uint16_t col = *indata++;

//        CrocColour incol = croc_colour_unpack_xrgb1555(col);
//        //CrocColour incol = croc_colour_unpack_rgb565(col);
//        //CrocColour incol = croc_colour_unpack_rgbx5551(col);
//        //incol = swapredblue(incol);
        *outdata++ = croc_colour_pack_rgba8888(incol);
    }

    *_out = out;
    return 0;
}

int croc_psx_texture_read_many(FILE *f, CrocPSXTexture **texture, int decompress)
{
    uint32_t           size;
    uint16_t           num_reserved0, num_reserved1;
    CrocPSXTextureType type;
    CrocPSXTexture     *_texture;
    int                ret;

    size = vsc_fread_leu16(f);
    if(feof(f) || ferror(f))
        return VSC_ERROR(EIO);

    if(size == 4) {
        size = 0;
        type = CROC_PSX_TEXTURE_UNCOMPRESSED4;
    } else {
        uint16_t tmp;

        tmp = vsc_fread_leu16(f);
        if(feof(f) || ferror(f))
            return VSC_ERROR(EIO);

        size = (tmp << 16) | size;

        type = vsc_fread_leu16(f);

        if(feof(f) || ferror(f))
            return VSC_ERROR(EIO);
    }

    switch(type) {
        case CROC_PSX_TEXTURE_UNCOMPRESSED4:
        case CROC_PSX_TEXTURE_UNCOMPRESSED:
        case CROC_PSX_TEXTURE_RLE16:
            break;

        default:
            return VSC_ERROR(EINVAL);
    }

    if((_texture = vsc_calloc(1, sizeof(CrocPSXTexture))) == NULL)
        return VSC_ERROR(ENOMEM);

    croc_psx_texture_init(_texture);

    _texture->type = type;

    num_reserved0 = vsc_fread_uint8(f);
    if(feof(f) || ferror(f)) {
        ret = VSC_ERROR(EIO);
        goto fail;
    }

    if(num_reserved0 > 0) {
        vsc_fseeko(f, sizeof(uint16_t) * num_reserved0, SEEK_CUR);

    }

    _texture->num_rects = vsc_fread_leu16(f);
    _texture->num_pages = vsc_fread_leu16(f);
    num_reserved1 = vsc_fread_leu16(f);
    if(num_reserved1 > 0) {
        assert(0);
        vsc_fseeko(f, 8 * _texture->num_pages, SEEK_CUR);
    }

    if(_texture->type != CROC_PSX_TEXTURE_UNCOMPRESSED4) {
        _texture->num_anims = vsc_fread_leu16(f);

        assert(_texture->num_anims == 0);
    }

    if((_texture->rects = vsc_calloc(_texture->num_rects, sizeof(CrocPSXTextureRect))) == NULL) {
        ret = VSC_ERROR(ENOMEM);
        goto fail;
    }

    if((_texture->pages = vsc_calloc(_texture->num_pages, sizeof(CrocPSXTexturePage))) == NULL) {
        ret = VSC_ERROR(ENOMEM);
        goto fail;
    }

    for(size_t i = 0; i < _texture->num_rects; ++i) {
        if((ret = read_rect(f, _texture->rects + i, _texture->type == CROC_PSX_TEXTURE_UNCOMPRESSED4)) < 0)
            goto fail;
    }

    for(size_t i = 0; i < _texture->num_pages; ++i) {
        if((ret = read_page(f, _texture->pages + i, _texture->type, 1)) < 0)
            goto fail;
    }


    {
        CrocTexture *ttttt[4096];
        size_t      n;
        croc_psx_tex_burst(_texture, ttttt, &n);

        char buf[128];


        for(size_t i = 0; i < n; ++i) {
            CrocTexture *tt = ttttt[i];
            croc_texture_rgba8888_to_rgba8888_arr(tt);

            snprintf(buf, sizeof(buf) - 1, "font/FONT_%03zu.png", i);
            buf[sizeof(buf) - 1] = '\0';

            stbi_write_png(buf, tt->width, tt->height, 4, tt->data, tt->bytes_per_row);

        }

    }
    {
        CrocTexture *tex;
        croc_psx_tex_to_tex(_texture, 0, &tex);

        //croc_texture_xrgb1555_to_rgb565(tex);
//
//        CrocTexture *tex2 = croc_texture_rgb565_to_rgba8888(tex, NULL);
        croc_texture_rgba8888_to_rgba8888_arr(tex);

        stbi_write_png("font.png", tex->width, tex->height, 4, tex->data, tex->bytes_per_row);
    }
    return 0;

fail:
    errno = VSC_UNERROR(ret);
    return ret;

}

int psxtex_convert(int argc, char **argv)
{

    FILE *f = fopen("slus/FONT.BIN", "rb");
    //FILE *f = fopen("slus/TPAGE213.BIN", "rb");

    CrocPSXTexture *tex;
    croc_psx_texture_read_many(f, &tex, 1);

    return 0;
}
