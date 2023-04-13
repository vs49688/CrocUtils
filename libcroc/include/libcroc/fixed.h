/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_FIXED_H
#define _LIBCROC_FIXED_H

#include "fixeddef.h"

static inline croc_x1616_t croc_float_to_x1616(float val)
{
    croc_x1616_t x = {(int32_t)(val * CROC_X1616_ONE)};
    return x;
}

static inline croc_x2012_t croc_float_to_x2012(float val)
{
    croc_x2012_t x = {(int32_t)(val * CROC_X2012_ONE)};
    return x;
}

static inline croc_x0412_t croc_float_to_x0412(float val)
{
    croc_x0412_t x = {(int16_t)(val * CROC_X0412_ONE)};
    return x;
}

static inline float croc_x1616_to_float(croc_x1616_t val)
{
    return val.v / CROC_X1616_ONE;
}

static inline float croc_x2012_to_float(croc_x2012_t val)
{
    return val.v / CROC_X2012_ONE;
}

static inline float croc_x0412_to_float(croc_x0412_t val)
{
    return val.v / CROC_X0412_ONE;
}

#endif /* LIBCROC_FIXED_H */
