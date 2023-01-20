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
