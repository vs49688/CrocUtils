/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
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
