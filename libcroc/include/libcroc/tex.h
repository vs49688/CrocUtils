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

#ifndef _LIBCROC_TEX_H
#define _LIBCROC_TEX_H

#include "coldef.h"
#include "texdef.h"

#ifdef __cplusplus
extern "C" {
#endif

CrocTexture *croc_texture_allocate(uint16_t width, uint16_t height, CrocTextureFormat format);
void         croc_texture_free(CrocTexture *texture);
int          croc_texture_read_many(FILE *f, CrocTexture **textures, size_t *num);
void         croc_texture_free_many(CrocTexture **textures, size_t num);

/*
 * Convert a RGB565 texture to RGBA8888.
 *
 * If key is non-NULL, the given RGB value is made fully transparent.
 *
 * Returns a new texture object.
 */
CrocTexture *croc_texture_rgb565_to_rgba8888(const CrocTexture *tex, const CrocColour *key);
CrocTexture *croc_texture_rgb888_to_rgba8888(const CrocTexture *tex, const CrocColour *key);
int          croc_texture_xrgb1555_to_rgb565(CrocTexture *tex);

/*
 * Convert a 8-bit indexed texture to RGBA8888.
 * It is expected the palette is XRGB8888.
 *
 * If key is non-NULL, the given RGB value is made fully transparent.
 *
 * Returns a new texture object.
 */
CrocTexture *croc_texture_deindex8(const CrocTexture *tex, const CrocColour *key);

int croc_texture_rgba8888_to_rgba8888_arr(CrocTexture *tex);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_TEX_H */
