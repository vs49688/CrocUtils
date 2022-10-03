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
    .type           = CROC_PSX_TEXTURE_UNCOMPRESSED,
    .num_rects      = 0,
    .num_tpage      = 0,
    .num_anims      = 0,
};

void croc_psx_texture_init(CrocPSXTexture *texture)
{
    *texture = default_texture;
}

static int read_rect(FILE *f, CrocPSXTextureRect *rect)
{
    uint8_t buf[CROC_PSX_TEXTURE_RECT_SIZE];

    if(fread(buf, CROC_PSX_TEXTURE_RECT_SIZE, 1, f) != 1)
        return VSC_ERROR(EIO);

    rect->unk_0 = vsc_read_le16(buf + 0);
    rect->unk_2 = vsc_read_le16(buf + 2);

    rect->uv_bl[0] = vsc_read_uint8(buf + 4);
    rect->uv_bl[1] = vsc_read_uint8(buf + 5);

    rect->uv_br[0] = vsc_read_uint8(buf + 6);
    rect->uv_br[1] = vsc_read_uint8(buf + 7);

    rect->uv_tl[0] = vsc_read_uint8(buf + 8);
    rect->uv_tl[1] = vsc_read_uint8(buf + 9);

    rect->uv_tr[0] = vsc_read_uint8(buf + 10);
    rect->uv_tr[1] = vsc_read_uint8(buf + 11);

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

int croc_psx_tex_to_tex(const CrocPSXTexture *tex, CrocTexture **_out)
{
    CrocTexture *out;
    uint16_t    *indata;
    uint32_t    *outdata;

    if(tex == NULL || _out == NULL)
        return VSC_ERROR(EINVAL);

    if((out = croc_texture_allocate(256, 256, CROC_TEXFMT_RGBA8888)) == NULL)
        return VSC_ERROR(errno);

    indata  = (uint16_t *)tex->data;
    outdata = out->data;

    for(int i = 0; i < 256 * 256; ++i) {
        uint16_t col = *indata++;

        //CrocColour incol = croc_colour_unpack_xrgb1555(col);
        //CrocColour incol = croc_colour_unpack_rgb565(col);
        CrocColour incol = croc_colour_unpack_rgbx5551(col);
        //incol = swapredblue(incol);
        *outdata++ = croc_colour_pack_rgba8888(incol);
    }

    *_out = out;
    return 0;
}

int croc_psx_texture_read_many(FILE *f, CrocPSXTexture **texture)
{
    uint32_t           size;
    uint16_t           num_reserved0, num_reserved1;
    CrocPSXTextureType type;
    CrocPSXTexture     *_texture;
    int                ret;

    size = vsc_fread_leu32(f);
    type = vsc_fread_leu16(f);

    if(feof(f) || ferror(f))
        return VSC_ERROR(EIO);

    if(type != CROC_PSX_TEXTURE_UNCOMPRESSED && type != CROC_PSX_TEXTURE_RLE16)
        return VSC_ERROR(EINVAL);

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
    _texture->num_tpage = vsc_fread_leu16(f);
    num_reserved1 = vsc_fread_leu16(f);
    if(num_reserved1 > 0) {
        assert(0);
        vsc_fseeko(f, 8 * _texture->num_tpage, SEEK_CUR);
    }
    _texture->num_anims = vsc_fread_leu16(f);

    assert(_texture->num_anims == 0);

    if((_texture->rects = vsc_calloc(_texture->num_rects, sizeof(CrocPSXTextureRect))) == NULL) {
        ret = VSC_ERROR(ENOMEM);
        goto fail;
    }

    for(size_t i = 0; i < _texture->num_rects; ++i) {
        if((ret = read_rect(f, _texture->rects + i)) < 0)
            goto fail;
    }


    if(_texture->type == CROC_PSX_TEXTURE_UNCOMPRESSED) {
        if(fread(&_texture->data, CROC_PSX_TEXTURE_TPAGE_SIZE, 1, f) != 1) {
            ret = VSC_ERROR(EIO);
            goto fail;
        }
    } else if(_texture->type == CROC_PSX_TEXTURE_RLE16) {
        uint32_t data_size;

        data_size = vsc_fread_leu32(f);
        if(feof(f) || ferror(f)) {
            ret = VSC_ERROR(EIO);
            goto fail;
        }

        static uint8_t bbbb[0x20000];
        if(fread(bbbb, data_size, 1, f) != 1) {
            ret = VSC_ERROR(EIO);
            goto fail;
        }

        if(croc_psx_texture_decompress(_texture->_data, bbbb, data_size, sizeof(_texture->_data)) < 0) {
            ret = VSC_ERROR(errno);
            goto fail;
        }

        _texture->type = CROC_PSX_TEXTURE_UNCOMPRESSED;
    }

    {
        CrocTexture *tex;
        croc_psx_tex_to_tex(_texture, &tex);

        //croc_texture_xrgb1555_to_rgb565(tex);
//
//        CrocTexture *tex2 = croc_texture_rgb565_to_rgba8888(tex, NULL);
        croc_texture_rgba8888_to_rgba8888_arr(tex);

        stbi_write_png("tpage213.png", tex->width, tex->height, 4, tex->data, tex->bytes_per_row);
    }
    return 0;
fail:

    return ret;

}

int psxtex_convert(int argc, char **argv)
{
    FILE *f = fopen("slus/FONT.BIN", "rb");

    CrocPSXTexture *tex;
    croc_psx_texture_read_many(f, &tex);

    return 0;
}
