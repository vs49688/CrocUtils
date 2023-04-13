/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_MOD_H
#define _LIBCROC_MOD_H

#include "moddef.h"

#ifdef __cplusplus
extern "C" {
#endif

CrocModel * croc_mod_init(CrocModel *m);
void        croc_mod_free(CrocModel *m);
void        croc_mod_free_many(CrocModel *m, size_t count);
int         croc_mod_read_many(FILE *f, CrocModel **models, size_t *num, CrocModelType type);
int         croc_mod_validate(const CrocModel *m, void(*proc)(void *, const char*, ...), void *user);

/*
 * Sort the model faces.
 * Currently just does it by material name in code-point order.
 */
void        croc_mod_sort_faces(CrocModel *m);
int         croc_mod_write_obj(FILE *f, const CrocModel *m, int usemtl);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_MOD_H */
