/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_CHUNK_H
#define _LIBCROC_CHUNK_H

#include <stddef.h>
#include <stdio.h>
#include "chunkdef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*CrocChunkEnumerator)(CrocChunkType type, const uint8_t *buf, size_t data, void *user);

int croc_chunk_enumerate(FILE *f, CrocChunkEnumerator proc, void *user);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_CHUNK_H */
