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