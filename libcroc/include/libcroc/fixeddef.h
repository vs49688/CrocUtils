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