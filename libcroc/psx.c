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
#include <libcroc/psx.h>

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
        rect->unk_4 = vsc_read_le16(buf + 4);
    else
        rect->unk_4 = 0;

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


int croc_psx_texture_read(FILE *f, CrocPSXTexture **texture, int decompress)
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

    _texture->_size = size;
    _texture->type  = type;

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
        if((ret = read_page(f, _texture->pages + i, _texture->type, decompress)) < 0)
            goto fail;
    }

    if(decompress)
        _texture->type = CROC_PSX_TEXTURE_UNCOMPRESSED;

    _texture->_checksum = vsc_read_leu32(f);
    if(feof(f) || ferror(f)) {
        ret = VSC_ERROR(EIO);
        goto fail;
    }

    *texture = _texture;

    return 0;

fail:
    croc_psx_texture_free(_texture);
    errno = VSC_UNERROR(ret);
    return ret;

}

void croc_psx_texture_free(CrocPSXTexture *tex)
{
    if(tex == NULL)
        return;

    if(tex->pages != NULL)
        vsc_free(tex->pages);

    if(tex->rects != NULL)
        vsc_free(tex->rects);

    vsc_free(tex);
}
