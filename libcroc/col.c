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
#include <errno.h>
#include <vsclib.h>
#include <libcroc/col.h>

CrocColour *croc_colour_read(void *p, CrocColour *c)
{
    uint8_t *d = p;
    c->r   = vsc_read_uint8(d + 0);
    c->g   = vsc_read_uint8(d + 1);
    c->b   = vsc_read_uint8(d + 2);
    c->pad = vsc_read_uint8(d + 3);
    return c;
}


void croc_colour_write(void *p, const CrocColour *c)
{
    uint8_t *d = p;
    vsc_write_uint8(d + 0, c->r);
    vsc_write_uint8(d + 1, c->g);
    vsc_write_uint8(d + 2, c->b);
    vsc_write_uint8(d + 3, c->pad);
}


CrocColour *croc_colour_fread(FILE *f, CrocColour *c)
{
    uint8_t buf[CROC_COLOUR_SIZE];
    if(fread(buf, CROC_COLOUR_SIZE, 1, f) != 1) {
        errno = EIO;
        return NULL;
    }

    return croc_colour_read(buf, c);
}

int croc_colour_fwrite(FILE *f, const CrocColour *c)
{
    uint8_t buf[CROC_COLOUR_SIZE];
    croc_colour_write(buf, c);

    if(fwrite(buf, sizeof(buf), 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    return 0;
}

CrocColour croc_colour_unpack_rgb565(uint16_t pixel)
{
    CrocColour c;

    /* Going for readability, the compiler should be able to combine the shifts. */
    c.r   = (uint8_t)((pixel & 0b1111100000000000u) >> 11u) << 3u;
    c.g   = (uint8_t)((pixel & 0b0000011111100000u) >>  5u) << 2u;
    c.b   = (uint8_t)((pixel & 0b0000000000011111u) >>  0u) << 3u;
    c.pad = 0xFFu;

    return c;
}

CrocColour croc_colour_unpack_xrgb1555(uint16_t pixel)
{
    CrocColour c;
    c.r   = (uint8_t)((pixel & 0b0111110000000000u) >> 10u) << 3u;
    c.g   = (uint8_t)((pixel & 0b0000001111100000u) >>  5u) << 3u;
    c.b   = (uint8_t)((pixel & 0b0000000000011111u) >>  0u) << 3u;
    c.pad = 0xFFu;
    return c;
}

CrocColour croc_colour_unpack_rgb888(const uint8_t *pixel)
{
    CrocColour c;
    c.r = *(pixel + 0);
    c.g = *(pixel + 1);
    c.b = *(pixel + 2);
    c.pad = 0xFFu;
    return c;
}

CrocColour croc_colour_unpack_xrgb8888(uint32_t pixel)
{
    CrocColour c;
    c.r   = (uint8_t)((pixel & 0x00FF0000) >> 16u);
    c.g   = (uint8_t)((pixel & 0x0000FF00) >>  8u);
    c.b   = (uint8_t)((pixel & 0x000000FF) >>  0u);
    c.pad = 0xFFu;
    return c;
}

uint16_t croc_colour_pack_rgb565(CrocColour c)
{
    return ((c.r >> 3u) << 11u) |
           ((c.g >> 2u) <<  5u) |
           ((c.b >> 3u) <<  0u);
}

uint32_t croc_colour_pack_rgba8888(CrocColour c)
{
    return (c.r << 24u) | (c.g << 16u) | (c.b << 8u) | (c.pad << 0u);
}
