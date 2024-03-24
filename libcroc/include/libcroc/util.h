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
