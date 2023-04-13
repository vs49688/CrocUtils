/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_COL_H
#define _LIBCROC_COL_H

#include <stdio.h>
#include "coldef.h"

#ifdef __cplusplus
extern "C" {
#endif

CrocColour *croc_colour_read(void *p, CrocColour *c);
void        croc_colour_write(void *p, const CrocColour *c);

CrocColour *croc_colour_fread(FILE *f, CrocColour *c);
int         croc_colour_fwrite(FILE *f, const CrocColour *c);

CrocColour  croc_colour_unpack_rgb565(uint16_t pixel);
CrocColour  croc_colour_unpack_xrgb1555(uint16_t pixel);
CrocColour  croc_colour_unpack_rgb888(const uint8_t *pixel);
CrocColour  croc_colour_unpack_xrgb8888(uint32_t pixel);
uint16_t    croc_colour_pack_rgb565(CrocColour c);
uint32_t    croc_colour_pack_rgba8888(CrocColour c);

#ifdef __cplusplus
}
#endif

#endif /* _COLCROC_VEC_H */
