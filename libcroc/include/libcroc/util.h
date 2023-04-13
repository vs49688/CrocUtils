/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_UTIL_H
#define _LIBCROC_UTIL_H

#include <stdint.h>
#include "utildef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Get a pointer to the filename of a path. */
const char *croc_util_get_filename(const char *path);

/*
 * Rip the level+sublevel from the filename (if possible)
 * MPXXX_YY.MAP -> XXX, YY
 */
int croc_extract_level_info(const char *path, uint16_t *level, uint16_t *sublevel);


#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_UTIL_H */
