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
#ifndef _LIBCROC_CHUNKDEF_H
#define _LIBCROC_CHUNKDEF_H

#include <stdint.h>

typedef enum CrocChunkFileType {
    CROC_FILE_TYPE_PIXELMAP = 2,
} CrocChunkFileType;

typedef enum CrocChunkType {
    CROC_CHUNK_PIXELMAP       =  2,
    CROC_CHUNK_TYPE_HEADER    = 18,
    CROC_CHUNK_TYPE_PIXELDATA = 33,
    CROC_CHUNK_TYPE_MATERIAL  = 60,
    CROC_CHUNK_TYPE_PIXELMAP2 = 61,
} CrocChunkType;

#endif /* _LIBCROC_CHUNKDEF_H */
