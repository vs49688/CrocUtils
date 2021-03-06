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
#ifndef _LIBCROC_TEXDEF_H
#define _LIBCROC_TEXDEF_H

#include <stdint.h>

#define CROC_TEXTURE_MAX_COUNT 256

typedef enum CrocTextureFormat {
    /*
     * 8-bit indexed colour.
     *
     * OpenGL:
     * - Don't even try. glColorTable() was removed in
     *   3.1 core. Convert to a non-indexed format.
     */
    CROC_TEXFMT_INDEX8     = 3,

    /*
     * XRGB1555, host native.
     *
     * OpenGL:
     * - Don't even try, do an in-place convert to
     *   RGB565 using croc_texture_xrgb1555_to_rgb565().
     */
    CROC_TEXFMT_XRGB1555   = 4,

    /*
     * RGB565, host-native
     *
     * OpenGL:
     * - internalFormat = GL_RGB
     * - format         = GL_RGB
     * - type           = GL_UNSIGNED_SHORT_5_6_5
     */
    CROC_TEXFMT_RGB565   = 5,

    /*
     * RGB888, host-native.
     *
     * OpenGL: annoying to deal with, convert to CROC_TEXFMT_RGBA8888
     */
    CROC_TEXFMT_RGB888  = 6,

    /*
     * XRGB8888, host-native
     *
     * OpenGL:
     * - Convert to CROC_TEXFMT_RGBA8888
     */
    CROC_TEXFMT_XRGB8888 = 7,

    /*
     * RGBA8888, host-native
     *
     * OpenGL:
     * - internalFormat = GL_RGBA
     * - format         = GL_RGBA
     * - type           = GL_UNSIGNED_INT_8_8_8_8
     */
    CROC_TEXFMT_RGBA8888 = 8,

    /*
     * RGBA8888, byte array (big-endian)
     *
     * Special custom format, not actually
     * used in Croc.
     *
     * OpenGL:
     * - internalFormat = GL_RGBA
     * - format         = GL_RGBA
     * - type           = GL_UNSIGNED_BYTE
     */
    CROC_TEXFMT_RGBA8888_ARR = 16384,
} CrocTextureFormat;

struct CrocTexture;
typedef struct CrocTexture CrocTexture;

struct CrocTexture {
    CrocTextureFormat format;
    uint16_t          bytes_per_row;
    uint16_t          width;
    uint16_t          height;
    int16_t           xorigin;
    int16_t           yorigin;
    /* Colour Palette, only if an indexed format. */
    CrocTexture       *palette;
    char              *name;
    void              *data;
};

#endif /* _LIBCROC_TEXDEF_H */
