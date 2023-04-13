/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <stdlib.h>
#include <vsclib.h>

#include <libcroc/fixed.h>
#include <libcroc/map.h>
#include <libcroc/util.h>

const char * const CrocMapFmtStrings[] = {
    [CROC_MAP_FMT_NORMAL]            = "normal",
    [CROC_MAP_FMT_V12]               = "v12",
    [CROC_MAP_FMT_V12_OLD]           = "v12_old",
    [CROC_MAP_FMT_V12_OLD_NOAMBIENT] = "v12_old_noambient",
    [CROC_MAP_FMT_INVALID]           = "invalid",
};

const char * const CrocMapStyleStrings[] = {
    [CROC_MAP_STYLE_ICE]      = "ice",
    [CROC_MAP_STYLE_WATER]    = "water",
    [CROC_MAP_STYLE_CASTLE]   = "castle",
    [CROC_MAP_STYLE_DESERT]   = "desert",
    [CROC_MAP_STYLE_CAVE]     = "cave",
    [CROC_MAP_STYLE_WOOD]     = "wood",
    [CROC_MAP_STYLE_DUNGEON]  = "dungeon",
    [CROC_MAP_STYLE_BOSS1]    = "boss1",
    [CROC_MAP_STYLE_BOSS2]    = "boss2",
    [CROC_MAP_STYLE_BOSS3]    = "boss3",
    [CROC_MAP_STYLE_BOSS4]    = "boss4",
    [CROC_MAP_STYLE_BOSS5]    = "boss5",
    [CROC_MAP_STYLE_BOSS6]    = "boss6",
    [CROC_MAP_STYLE_BOSS7]    = "boss7",
    [CROC_MAP_STYLE_BOSS8]    = "boss8",
    [CROC_MAP_STYLE_BOSS9]    = "boss9",
    [CROC_MAP_STYLE_WHALE]    = "whale",
    [CROC_MAP_STYLE_ISLAND1]  = "island1",
    [CROC_MAP_STYLE_ISLAND2]  = "island2",
    [CROC_MAP_STYLE_ISLAND3]  = "island3",
    [CROC_MAP_STYLE_ISLAND4]  = "island4",
    [CROC_MAP_STYLE_ISLAND5]  = "island5",
    [CROC_MAP_STYLE_ICESLIDE] = "iceslide",
    [CROC_MAP_STYLE_BOULDER]  = "boulder",
    [CROC_MAP_STYLE_ICECAVE]  = "icecave",
};

const char * const CrocMapEffectStrings[] = {
    [CROC_MAP_EFFECT_NONE]                = "none",
    [CROC_MAP_EFFECT_SNOW]                = "snow",
    [CROC_MAP_EFFECT_FOG]                 = "fog",
    [CROC_MAP_EFFECT_TORCH]               = "torch",
    [CROC_MAP_EFFECT_TORCH_LIGHTNING]     = "torch_lightning",
    [CROC_MAP_EFFECT_TORCH_BIG]           = "torch_big",
    [CROC_MAP_EFFECT_TORCH_BIG_LIGHTNING] = "torch_big_lightning",
    [CROC_MAP_EFFECT_DARK]                = "dark",
    [CROC_MAP_EFFECT_SNOW_SMALL]          = "snow_small",
    [CROC_MAP_EFFECT_SNOW_ISLAND]         = "snow_island",
    [CROC_MAP_EFFECT_TORCH_GIANT]         = "torch_giant",
};

const char * const CrocMapAmbienceStrings[] = {
    [CROC_MAP_AMBI_NONE] = "none",
    [CROC_MAP_AMBI_BIRD] = "bird",
    [CROC_MAP_AMBI_DRIP] = "drip",
    [CROC_MAP_AMBI_NGHT] = "nght",
    [CROC_MAP_AMBI_WATR] = "watr",
    [CROC_MAP_AMBI_CRWD] = "crwd",
    [CROC_MAP_AMBI_LAVA] = "lava",
    [CROC_MAP_AMBI_TUNL] = "tunl",
    [CROC_MAP_AMBI_ICE1] = "ice1",
    [CROC_MAP_AMBI_TOMB] = "tomb",
    [CROC_MAP_AMBI_DES1] = "des1",
    [CROC_MAP_AMBI_DCAV] = "dcav",
    [CROC_MAP_AMBI_SLDE] = "slde",
    [CROC_MAP_AMBI_CAST] = "cast",
    [CROC_MAP_AMBI_DNGN] = "dngn",
    [CROC_MAP_AMBI_ICE2] = "ice2",
    [CROC_MAP_AMBI_CLWD] = "clwd",
    [CROC_MAP_AMBI_WHAL] = "whal",
    [CROC_MAP_AMBI_MOAT] = "moat",
    [CROC_MAP_AMBI_DUSK] = "dusk",
    [CROC_MAP_AMBI_WIND] = "wind",
    [CROC_MAP_AMBI_CICA] = "cica",
    [CROC_MAP_AMBI_DANT] = "dant",
};

const char * const CrocMapDoorFlagStrings[] = {
    [CROC_MAP_DOOR_NORMAL]  = "normal",
    [CROC_MAP_DOOR_FLOOR]   = "floor",
    [CROC_MAP_DOOR_CEILING] = "ceiling",
    [CROC_MAP_DOOR_START]   = "start",
    [CROC_MAP_DOOR_FINAL]   = "final",
};

const CrocWad CrocMapStyleToWad[] = {
    [CROC_MAP_STYLE_ICE]      = CROC_WAD_MATERIAL_ICE,
    [CROC_MAP_STYLE_WATER]    = CROC_WAD_MATERIAL_WATER,
    [CROC_MAP_STYLE_CASTLE]   = CROC_WAD_MATERIAL_CASTLE,
    [CROC_MAP_STYLE_DESERT]   = CROC_WAD_MATERIAL_DESERT,
    [CROC_MAP_STYLE_CAVE]     = CROC_WAD_MATERIAL_CAVE,
    [CROC_MAP_STYLE_WOOD]     = CROC_WAD_MATERIAL_FOREST,
    [CROC_MAP_STYLE_DUNGEON]  = CROC_WAD_MATERIAL_DUNGEON,
    [CROC_MAP_STYLE_BOSS1]    = CROC_WAD_MATERIAL_BOSS1,
    [CROC_MAP_STYLE_BOSS2]    = CROC_WAD_MATERIAL_BOSS2,
    [CROC_MAP_STYLE_BOSS3]    = CROC_WAD_MATERIAL_BOSS3,
    [CROC_MAP_STYLE_BOSS4]    = CROC_WAD_MATERIAL_BOSS4,
    [CROC_MAP_STYLE_BOSS5]    = CROC_WAD_MATERIAL_BOSS5,
    [CROC_MAP_STYLE_BOSS6]    = CROC_WAD_MATERIAL_BOSS6,
    [CROC_MAP_STYLE_BOSS7]    = CROC_WAD_MATERIAL_BOSS7,
    [CROC_MAP_STYLE_BOSS8]    = CROC_WAD_MATERIAL_BOSS8,
    [CROC_MAP_STYLE_BOSS9]    = CROC_WAD_MATERIAL_BOSS9,
    [CROC_MAP_STYLE_WHALE]    = CROC_WAD_NONE,
    [CROC_MAP_STYLE_ISLAND1]  = CROC_WAD_MATERIAL_ISLAND1,
    [CROC_MAP_STYLE_ISLAND2]  = CROC_WAD_MATERIAL_ISLAND2,
    [CROC_MAP_STYLE_ISLAND3]  = CROC_WAD_MATERIAL_ISLAND3,
    [CROC_MAP_STYLE_ISLAND4]  = CROC_WAD_MATERIAL_ISLAND4,
    [CROC_MAP_STYLE_ISLAND5]  = CROC_WAD_MATERIAL_ISLAND5,
    [CROC_MAP_STYLE_ICESLIDE] = CROC_WAD_NONE,
    [CROC_MAP_STYLE_BOULDER]  = CROC_WAD_NONE,
    [CROC_MAP_STYLE_ICECAVE]  = CROC_WAD_MATERIAL_ICECAVE,
};

static const CrocMap default_map = {
    ._format        = CROC_MAP_FMT_NORMAL,
    ._version       = 21,
    ._level         = 0,
    ._sublevel      = 0,
    ._checksum      = 0,

    .name           = "Default New Map",

    .width          = 16,
    .height         = 16,
    .depth          = 32,
    .style          = CROC_MAP_STYLE_ICE,

    .ambient_colour = {
        .r = 64,
        .g = 64,
        .b = 64,
    }
};

void croc_map_init(CrocMap *map)
{
    if(map == NULL)
        return;

    *map = default_map;
}

int croc_map_rebase(CrocMap *map, int base)
{
    if(base == 0)
        return 0;

    /* Check we actually can rebase. */
    for(int i = 0; i < map->num_doors; ++i) {
        int newlevel = map->door[i].level + base;
        if(newlevel < 0 || newlevel > CROC_MAP_MAX_LEVEL)
            return -1;
    }

    /* Do it. */
    for(int i = 0; i < map->num_doors; ++i)
        map->door[i].level += base;

    return 0;
}
