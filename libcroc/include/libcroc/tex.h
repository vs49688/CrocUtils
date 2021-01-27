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

#include "texdef.h"

CrocTexture *croc_texture_allocate(uint16_t width, uint16_t height, CrocTextureFormat format);
void         croc_texture_free(CrocTexture *texture);
int          croc_texture_read_many(FILE *f, CrocTexture **textures, size_t *num);
void         croc_texture_free_many(CrocTexture **textures, size_t num);

/*
 * Convert a RGB565 texture to RGBA8888.
 * Returns a new texture object.
 */
CrocTexture *croc_texture_rgb565_to_rgba8888(const CrocTexture *tex);

#endif /* _LIBCROC_TEX_H */
