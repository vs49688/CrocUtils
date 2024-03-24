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
