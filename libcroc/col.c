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
