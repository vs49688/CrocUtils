/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <libcroc/checksum.h>

void croc_checksum_append(uint32_t *ck, void *p, size_t size)
{
    uint8_t *d = p;

    for(size_t i = 0; i < size; ++i)
        *ck += d[i];
}

uint32_t croc_checksum(void *p, size_t size)
{
    uint32_t ck = 0;
    croc_checksum_append(&ck, p, size);
    return ck;
}
