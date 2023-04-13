/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_CHUNKDEF_H
#define _LIBCROC_CHUNKDEF_H

#include <stdint.h>

typedef enum CrocChunkFileType {
    CROC_FILE_TYPE_PIXELMAP = 2,
} CrocChunkFileType;

typedef enum CrocChunkType {
    CROC_CHUNK_PIXELMAP       =  3,
    CROC_CHUNK_TYPE_HEADER    = 18,
    CROC_CHUNK_TYPE_PIXELDATA = 33,
    /*
     * A "glue" chunk. "Glues" the palette to the index.
     *
     * This needs a better name.
     */
    CROC_CHUNK_TYPE_PAL_GLUE  = 34,
    CROC_CHUNK_TYPE_MATERIAL  = 60,
    CROC_CHUNK_TYPE_PIXELMAP2 = 61,
} CrocChunkType;

#endif /* _LIBCROC_CHUNKDEF_H */
