/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_FIXEDDEF_H
#define _LIBCROC_FIXEDDEF_H

#include <stdint.h>

const static float CROC_X1616_ONE = 65536.0f; /* 1 << 16 */
const static float CROC_X2012_ONE =  4096.0f; /* 1 << 12 */
const static float CROC_X0412_ONE =  4096.0f; /* 1 << 12 */

/* Fixed-point types. Are structs to prevent simple assignment. */
typedef struct { int32_t v; } croc_x1616_t;
typedef struct { int32_t v; } croc_x2012_t;
typedef struct { int16_t v; } croc_x0412_t;

#endif /* _LIBCROC_FIXEDDEF_H */
