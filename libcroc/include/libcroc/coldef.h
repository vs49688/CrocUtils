/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_COLDEF_H
#define _LIBCROC_COLDEF_H

#include <stdint.h>

#define CROC_COLOUR_SIZE 4

typedef struct CrocColour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pad;
} CrocColour;

#endif /* _LIBCROC_COLDEF_H */
