/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_MAP_H
#define _LIBCROC_MAP_H

#include <stdio.h>
#include "mapdef.h"
#include "waddef.h"

struct cJSON;
typedef struct cJSON cJSON;

extern const char * const CrocMapFmtStrings[];
extern const char * const CrocMapStyleStrings[];
extern const char * const CrocMapEffectStrings[];
extern const char * const CrocMapAmbienceStrings[];
extern const char * const CrocMapDoorFlagStrings[];
extern const CrocWad CrocMapStyleToWad[];

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
