/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_DIR_H
#define _LIBCROC_DIR_H

#include <stdio.h>
#include "dirdef.h"

struct cJSON;
typedef struct cJSON cJSON;

#ifdef __cplusplus
extern "C" {
#endif

int croc_dir_read(FILE *f, CrocDirEntry **entry, size_t *count, int *old);

int croc_dir_write(FILE *f, const CrocDirEntry *entries, size_t count, int big);

cJSON *croc_dir_write_json(const CrocDirEntry *entries, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_DIR_H */
