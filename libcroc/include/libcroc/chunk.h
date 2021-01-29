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
#ifndef _LIBCROC_CHUNK_H
#define _LIBCROC_CHUNK_H

#include <stddef.h>
#include <stdio.h>
#include "chunkdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*CrocChunkEnumerator)(CrocChunkType type, const uint8_t *buf, size_t data, void *user);

int croc_chunk_enumerate(FILE *f, CrocChunkEnumerator proc, void *user);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_CHUNK_H */
