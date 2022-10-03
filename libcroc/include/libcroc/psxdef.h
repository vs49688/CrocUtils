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
#ifndef _LIBCROC_PSXDEF_H
#define _LIBCROC_PSXDEF_H

#include <stdint.h>

#define CROC_PSX_TEXTURE_MAX_RESERVED0  8
#define CROC_PSX_TEXTURE_RECT4_SIZE     14
#define CROC_PSX_TEXTURE_RECT_SIZE      12
#define CROC_PSX_TEXTURE_TPAGE_SIZE     0x20000 /* 256*256*sizeof(uint16_t) */

typedef enum CrocPSXTextureType {
    CROC_PSX_TEXTURE_UNCOMPRESSED4 = 4,
    CROC_PSX_TEXTURE_UNCOMPRESSED  = 5,
    CROC_PSX_TEXTURE_RLE16         = 6,
} CrocPSXTextureType;

typedef struct CrocPSXAnimationBlock {
    uint16_t num_frames;
    uint16_t width; /* In halfword units */
    uint16_t height;
    uint16_t unk_6;
    uint16_t unk_8;
    uint16_t unk_10;
    uint8_t  *data; /* num_frames * width * height * 2 */
} CrocPSXAnimationBlock;

typedef struct CrocPSXTextureRect {
    uint16_t unk_0;
    uint16_t unk_2;
    uint16_t unk_4; // Only in CROC_PSX_TEXTURE_UNCOMPRESSED4

    /*
     * UV offsets within a PSX texture page? [0, 255]
     */
    uint8_t uv_tl[2]; /* Top-Left     */
    uint8_t uv_bl[2]; /* Bottom-Left  */
    uint8_t uv_tr[2]; /* Top-Right    */
    uint8_t uv_br[2]; /* Bottom-Right */
} CrocPSXTextureRect;

typedef struct CrocPSXTexturePage {
    /*
     * Size of the data, if compressed.
     * Will be CROC_PSX_TEXTURE_TPAGE_SIZE otherwise.
     */
    uint32_t data_size;

    union {
        uint8_t data[CROC_PSX_TEXTURE_TPAGE_SIZE];
        uint8_t _data[CROC_PSX_TEXTURE_TPAGE_SIZE + 2];
    };
} CrocPSXTexturePage;

typedef struct CrocPSXTexture {
    uint32_t _checksum;

    CrocPSXTextureType type;

    uint16_t num_rects;

    uint16_t num_pages;

    uint16_t num_anims;

    CrocPSXTextureRect *rects;

    CrocPSXTexturePage *pages;
} CrocPSXTexture;

#endif /* _LIBCROC_PSXDEF_H */
