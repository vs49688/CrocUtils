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
#ifndef _LIBCROC_COL_H
#define _LIBCROC_COL_H

#include <stdio.h>
#include "coldef.h"

#ifdef __cplusplus
extern "C" {
#endif

CrocColour *croc_colour_read(void *p, CrocColour *c);
void        croc_colour_write(void *p, const CrocColour *c);

CrocColour *croc_colour_fread(FILE *f, CrocColour *c);
int         croc_colour_fwrite(FILE *f, const CrocColour *c);

CrocColour  croc_colour_unpack_rgb565(uint16_t pixel);
uint32_t    croc_colour_pack_rgba8888(CrocColour c);

#ifdef __cplusplus
}
#endif

#endif /* _COLCROC_VEC_H */
