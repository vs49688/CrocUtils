/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_STRUN_H
#define _LIBCROC_STRUN_H

#include <stdio.h>
#include "strundef.h"

struct cJSON;
typedef struct cJSON cJSON;

#ifdef __cplusplus
extern "C" {
#endif

CrocStRunEntry *croc_strun_read(FILE *f, size_t *count);

cJSON *croc_strun_write_json(const CrocStRunEntry *entries, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_STRUN_H */
