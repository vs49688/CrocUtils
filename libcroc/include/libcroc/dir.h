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
#ifndef _LIBCROC_DIR_H
#define _LIBCROC_DIR_H

#include "dirdef.h"

struct cJSON;
typedef struct cJSON cJSON;

#ifdef __cplusplus
extern "C" {
#endif

CrocDirEntry *croc_dir_read(FILE *f,  size_t *count, int *old);

cJSON *croc_dir_write_json(const CrocDirEntry *entries, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_DIR_H */