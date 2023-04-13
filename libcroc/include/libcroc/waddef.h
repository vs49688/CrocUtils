/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_WADDEF_H
#define _LIBCROC_WADDEF_H

#include <stdint.h>

typedef enum CrocWadFileType {
	CROC_WAD_FILE_TYPE_BINARY = 0,
	CROC_WAD_FILE_TYPE_MAP    = 1,
} CrocWadFileType;

typedef enum CrocWadType {
	CROC_WAD_TYPE_MPLOAD = 0,
	CROC_WAD_TYPE_MAPXX  = 1,
} CrocWadType;

typedef enum CrocWadRleType {
	CROC_WAD_RLE_BYTE = 0,
	CROC_WAD_RLE_WORD,
	CROC_WAD_RLE_NONE,
} CrocWadRleType;

typedef enum CrocWad {
    CROC_WAD_NONE,
    CROC_WAD_ANIMS,
    CROC_WAD_MAPS,
    CROC_WAD_OBJECTS,
    CROC_WAD_SOUND,
    CROC_WAD_TEXTURES,
    CROC_WAD_FONTS,
    CROC_WAD_MATERIAL,
    CROC_WAD_MATERIAL_ICE,
    CROC_WAD_MATERIAL_DESERT,
    CROC_WAD_MATERIAL_WATER,
    CROC_WAD_MATERIAL_CASTLE,
    CROC_WAD_MATERIAL_CAVE,
    CROC_WAD_MATERIAL_FOREST,
    CROC_WAD_MATERIAL_ICECAVE,
    CROC_WAD_MATERIAL_DUNGEON,
    CROC_WAD_MATERIAL_ISLAND1,
    CROC_WAD_MATERIAL_ISLAND2,
    CROC_WAD_MATERIAL_ISLAND3,
    CROC_WAD_MATERIAL_ISLAND4,
    CROC_WAD_MATERIAL_ISLAND5,
    CROC_WAD_MATERIAL_BOSS1,
    CROC_WAD_MATERIAL_BOSS2,
    CROC_WAD_MATERIAL_BOSS3,
    CROC_WAD_MATERIAL_BOSS4,
    CROC_WAD_MATERIAL_BOSS5,
    CROC_WAD_MATERIAL_BOSS6,
    CROC_WAD_MATERIAL_BOSS7,
    CROC_WAD_MATERIAL_BOSS8,
    CROC_WAD_MATERIAL_BOSS9,
    CROC_WAD_MATERIAL_INTRO,
    CROC_WAD_MATERIAL_INTROJIG,
    CROC_WAD_MATERIAL_OUTRO,
    CROC_WAD_PALETTES,
    CROC_WAD_STRATS,
    CROC_WAD_MAX = CROC_WAD_STRATS,
} CrocWad;

typedef struct CrocWadEntry {
	char           *filename;
	uint32_t       offset;
	uint32_t       compressed_size;
	uint32_t       uncompressed_size;
	CrocWadRleType rle_type;
} CrocWadEntry;

typedef struct CrocWadFs {
    char         *idx_path;
    char         *wad_path;
    CrocWadEntry *entries;
    size_t        num_entries;
    FILE         *wad;
} CrocWadFs;

#endif /* _LIBCROC_WADDEF_H */
