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
#ifndef _LIBCROC_VEC_H
#define _LIBCROC_VEC_H

#include <stdio.h>
#include "vecdef.h"

#ifdef __cplusplus
extern "C" {
#endif

CrocVector *croc_vector_read(void *p, CrocVector *v);
void        croc_vector_write(void *p, const CrocVector *v);

CrocVector *croc_vector_fread(FILE *f, CrocVector *v);
int         croc_vector_fwrite(FILE *f, const CrocVector *v);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_VEC_H */
