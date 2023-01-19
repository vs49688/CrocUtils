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
#ifndef _LIBCROC_WADDEF_H
#define _LIBCROC_WADDEF_H

#include <stdint.h>

typedef enum CrocWadFileType {
	CROC_WAD_FILE_TYPE_BINARY = 0,
	CROC_WAD_FILE_TYPE_MAP    = 1,
} CrocWadFileType;

typedef enum CrocWadType {
	CROC_WAD_TYPE_MPLOAD = 0,
	CROC_WAD_TYPE_MAPXX  = 1,
} CrocWadType;

typedef enum CrocWadRleType {
	CROC_WAD_RLE_BYTE = 0,
	CROC_WAD_RLE_WORD,
	CROC_WAD_RLE_NONE,
} CrocWadRleType;

typedef struct CrocWadEntry {
	char           *filename;
	uint32_t       offset;
	uint32_t       compressed_size;
	uint32_t       uncompressed_size;
	CrocWadRleType rle_type;
} CrocWadEntry;

#endif /* _LIBCROC_WADDEF_H */
