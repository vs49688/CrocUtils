/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_CHECKSUM_H
#define _LIBCROC_CHECKSUM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Calculate a checksum.
 * - If the block contains a leading size field, that
 *   is usually included too.
 * - Initial checksum value is 0.
 */
void     croc_checksum_append(uint32_t *ck, void *p, size_t size);
uint32_t croc_checksum(void *p, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_CHECKSUM_H */
