/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_VECDEF_H
#define _LIBCROC_VECDEF_H

#include <stdint.h>
#include "fixeddef.h"

#define CROC_VECTOR_SIZE 8

typedef struct CrocVector
{
    croc_x0412_t x;
    croc_x0412_t y;
    croc_x0412_t z;
    croc_x0412_t pad;
} CrocVector;

#endif /* _LIBCROC_VECDEF_H */
