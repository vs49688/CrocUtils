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

//static CrocColour swapredblue(CrocColour col)
//{
//    uint8_t tmp = col.r;
//    col.r = col.b;
//    col.b = tmp;
//    return col;
//}

CrocColour croc_colour_unpack_rgbx5551(uint16_t pixel)
{
    CrocColour c = croc_colour_unpack_xrgb1555((pixel >> 1) & 0x7FFF);


    return c;
}

void sadfasdfasd(const CrocPSXTextureRect *rect, const void *data, uint16_t *outdata)
{
    const uint8_t *page             = (uint8_t *)data;
    const size_t  src_height        = rect->uv_tr[1] - rect->uv_br[1];
    const size_t  src_width         = (rect->uv_tr[0] - rect->uv_tl[0]) * 2;
    const size_t  src_bytes_per_row = 512 * sizeof(uint8_t);


    page += (src_bytes_per_row * rect->uv_tl[1]) + (rect->uv_tl[0] * 2);
    for(size_t j = 0; j < src_height; ++j) {
        for(size_t i = 0; i < src_width; ++i) {
//            uint8_t idx = page[i];
//
//            CrocColour col = {
//                .r = idx,
//                .b = idx,
//                .g = idx,
//                .pad = 0xFF,
//            };

//            *outdata++ = croc_colour_pack_rgb565(col);
            *outdata++ = page[i];
        }

        page += src_bytes_per_row;
    }
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

int croc_psx_tex_to_tex16(const CrocPSXTexture *tex, size_t page, CrocTexture **_out)
{
    CrocTexture   *out;
    const uint16_t *indata;
    uint32_t      *outdata;

    if(tex == NULL || _out == NULL)
        return VSC_ERROR(EINVAL);

    if(page >= tex->num_pages)
        return VSC_ERROR(EINVAL);

    if((out = croc_texture_allocate(256, 256, CROC_TEXFMT_RGBA8888)) == NULL)
        return VSC_ERROR(errno);

    indata  = (const uint16_t *)tex->pages[page].data;
    outdata = out->data;


    for(int i = 0; i < 256 * 256; ++i) {
//        uint8_t idx = *indata++;
//
//        CrocColour incol = {
//            .r = idx,
//            .g = idx,
//            .b = idx,
//            .pad = 0xFF,
//        };
        uint16_t col = *indata++;

        CrocColour incol = croc_colour_unpack_xrgb1555(col);
//        //CrocColour incol = croc_colour_unpack_rgb565(col);
//        //CrocColour incol = croc_colour_unpack_rgbx5551(col);
//        //incol = swapredblue(incol);
        *outdata++ = croc_colour_pack_rgba8888(incol);
    }

    *_out = out;
    return 0;
}

__attribute__((unused)) static int asdfasd(CrocPSXTexture *tex, CrocTexture **_pal)
{
    const uint8_t *data = tex->pages[0].data;
    CrocTexture   *pal;

    if((pal = croc_texture_allocate(1, 256, CROC_TEXFMT_RGB565)) == NULL)
        return VSC_ERROR(ENOMEM);

    CrocPSXTextureRect rect = {
        .uv_tl = {0, 0},
        .uv_tr = {32, 0},
        .uv_bl = {0, 59},
        .uv_br = {32, 59},
    };

    sadfasdfasd(&rect, data, pal->data);

    *_pal = pal;
    return 0;
}

int psxtex_convert(int argc, char **argv)
{

    FILE *f = fopen("slus/FRONTEND.BIN", "rb");
    //FILE *f = fopen("slus/TPAGE213.BIN", "rb");

    CrocPSXTexture *tex;
    croc_psx_texture_read(f, &tex, 1);

    {
        CrocTexture *ttttt[4096];
        size_t      n;
        croc_psx_tex_burst(tex, ttttt, &n);

        char buf[128];


        for(size_t i = 0; i < n; ++i) {
            CrocTexture *tt = ttttt[i];
            croc_texture_rgba8888_to_rgba8888_arr(tt);

            snprintf(buf, sizeof(buf) - 1, "frontend/FRONTEND_%03zu.png", i);
            buf[sizeof(buf) - 1] = '\0';

            stbi_write_png(buf, tt->width, tt->height, 4, tt->data, tt->bytes_per_row);

        }

    }
    {
        CrocTexture *tex2;
        croc_psx_tex_to_tex16(tex, 0, &tex2);

        //croc_texture_xrgb1555_to_rgb565(tex);
//
//        CrocTexture *tex2 = croc_texture_rgb565_to_rgba8888(tex, NULL);
        croc_texture_rgba8888_to_rgba8888_arr(tex2);

        stbi_write_png("FRONTEND.png", tex2->width, tex2->height, 4, tex2->data, tex2->bytes_per_row);
    }

    return 0;
}
