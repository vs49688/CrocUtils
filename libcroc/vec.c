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
#include <errno.h>
#include <vsclib.h>
#include <libcroc/vec.h>

CrocVector *croc_vector_read(void *p, CrocVector *v)
{
    uint8_t *d = p;
    v->x.v   = vsc_read_le16(d + 0);
    v->y.v   = vsc_read_le16(d + 2);
    v->z.v   = vsc_read_le16(d + 4);
    v->pad.v = vsc_read_le16(d + 6);
    return v;
}

void croc_vector_write(void *p, const CrocVector *v)
{
    uint8_t *d = p;
    vsc_write_le16(d + 0, v->x.v);
    vsc_write_le16(d + 2, v->y.v);
    vsc_write_le16(d + 4, v->z.v);
    vsc_write_le16(d + 6, v->pad.v);
}

CrocVector *croc_vector_fread(FILE *f, CrocVector *v)
{
    uint8_t buf[CROC_VECTOR_SIZE];
    if(fread(buf, CROC_VECTOR_SIZE, 1, f) != 1) {
        errno = EIO;
        return NULL;
    }

    return croc_vector_read(buf, v);
}

int croc_vector_fwrite(FILE *f, const CrocVector *v)
{
    uint8_t buf[CROC_VECTOR_SIZE];
    croc_vector_write(buf, v);

    if(fwrite(buf, sizeof(buf), 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    return 0;
}

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
