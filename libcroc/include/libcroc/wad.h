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
#ifndef _LIBCROC_WAD_H
#define _LIBCROC_WAD_H

#include <stddef.h>
#include <stdio.h>
#include <cJSON.h>
#include "waddef.h"
#include "mapdef.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char * const CrocWadPaths[];

/* Is the file a Saturn MPLOAD%02u.WAD? */
int croc_wad_is_mpload(const char *path, unsigned int *level);

/* Is the file a PSX MAP%02u.WAD? */
int croc_wad_is_mapxx(const char *path, unsigned int *level);

int croc_wad_read_index(FILE *f, CrocWadEntry **entries, size_t *num);

void croc_wad_free_index(CrocWadEntry *entries, size_t num);

cJSON *croc_wad_index_write_json(const CrocWadEntry *entries, size_t count);

/* Decompress a byte-sized RLE.*/
int croc_wad_decompressb(void *outbuf, const void *inbuf, size_t compressed_size, size_t uncompressed_size);

/* Decompress a word-sized RLE. Output buffer must be a multiple of 2. */
int croc_wad_decompressw(void *outbuf, const void *inbuf, size_t compressed_size, size_t uncompressed_size);

int croc_wad_load_entry(FILE *f, const CrocWadEntry *entry, void **buf);

int croc_wadfs_open(CrocWadFs **fs, const char *base);

int croc_wadfs_load(CrocWadFs *fs, const char *name, void **data, const CrocWadEntry **const _entry);

void croc_wadfs_close(CrocWadFs *fs);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_WAD_H */
