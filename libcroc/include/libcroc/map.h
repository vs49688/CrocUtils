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
#ifndef _LIBCROC_MAP_H
#define _LIBCROC_MAP_H

#include <stdio.h>
#include "mapdef.h"

struct cJSON;
typedef struct cJSON cJSON;

extern const char * const CrocMapFmtStrings[];
extern const char * const CrocMapStyleStrings[];
extern const char * const CrocMapEffectStrings[];
extern const char * const CrocMapAmbienceStrings[];
extern const char * const CrocMapDoorFlagStrings[];

#ifdef __cplusplus
extern "C" {
#endif

void    croc_map_init(CrocMap *map);
void    croc_map_free(CrocMap *map);
int     croc_map_read(FILE *f, CrocMap *map);
int     croc_map_rebase(CrocMap *map, int base);
int     croc_map_write(FILE *f, const CrocMap *map);

cJSON   *croc_map_write_json(const CrocMap *map);
CrocMap *croc_map_read_json(const cJSON *j, CrocMap *map);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_MAP_H */