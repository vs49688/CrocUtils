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
#ifndef _LIBCROC_PSX_H
#define _LIBCROC_PSX_H

#include <stddef.h>
#include "psxdef.h"

#ifdef __cplusplus
extern "C" {
#endif

void croc_psx_texture_init(CrocPSXTexture *texture);

int croc_psx_texture_read(FILE *f, CrocPSXTexture **texture, int decompress);

void croc_psx_texture_free(CrocPSXTexture *texture);

int croc_psx_texture_decompress(void *_out, const void *_in, size_t compressed_size, size_t uncompressed_size);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_PSX_H */
