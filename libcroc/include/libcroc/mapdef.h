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
#ifndef _LIBCROC_MAPDEF_H
#define _LIBCROC_MAPDEF_H

#include "fixeddef.h"

/* Number of directional light structures in the file. */
#define CROC_MAP_MAX_DIRECT_LIGHTS    3
#define CROC_MAP_MAX_PARAMS           8
#define CROC_MAP_STRING_LEN         255
#define CROC_MAP_MAX_LEVEL          999
#define CROC_MAP_MAX_SUBLEVEL        99

/* On-disk sizes of some of the structures. */
#define CROC_MAP_INFO_SIZE          38
#define CROC_MAP_TRACK_SIZE         14
#define CROC_MAP_TRACK_SIZE_PREV21  10
#define CROC_MAP_WAYPOINT_SIZE      16
#define CROC_MAP_POINT_LIGHT_SIZE   24
#define CROC_MAP_DIRECT_LIGHT_SIZE  12
#define CROC_MAP_COLOUR_SIZE         4
#define CROC_MAP_BASE_STRAT_SIZE    50
#define CROC_MAP_DOOR_SIZE          14

typedef enum CrocMapFormat {
    /*
     * Standard map format, found in the PC/PSX versions of the game.
     * - Has a leading size field and trailing checksum field.
     * - Valid versions are 12, 20, and 21.
     * - There is no difference between version 12 and 20.
     * - Version 21 has two reserved fields in the track definition.
     */
    CROC_MAP_FMT_NORMAL             = 0,
    /*
     * v12 map format. Same as CROC_MAP_FMT_NORMAL except:
     * - The only valid version is 12.
     * - Uses the v12 strat format
     *
     * These are almost impossible to tell apart from CROC_MAP_FMT_NORMAL.
     */
    CROC_MAP_FMT_V12                = 1,
    /*
     * Old v12 map format, found in the 0.12 beta version of the game.
     * - No leading size field, may or may not have trailing checksum.
     * - The only valid version is 12.
     * - padding changed to a uint8
     * - Uses the v12 strat format
     */
    CROC_MAP_FMT_V12_OLD            = 2,
    /* Same as CROC_MAP_FMT_V12_OLD, except without the ambient colour. */
    CROC_MAP_FMT_V12_OLD_NOAMBIENT  = 3,
    CROC_MAP_FMT_INVALID            = 4,
} CrocMapFormat;

/*
 * Unsure about some of these.
 * - Most of these are in gdata/material
 * - BOSS5 is 11, so I'm guessing for BOSS[1-4,5-9]
 * - ICESLIDE is an educated guess. The internal filename
 *   of MP000_01.MAP and MP000_02.MAP from the beta are
 *   both ICESLIDE.MAP, with style == 22.
 * - BOULDER is also a guess. A lot of the maps with "Boulder"
 *   in the title and strats have style == 23.
 * - I have no idea what style 16 is.
 */
typedef enum CrocMapStyle {
    CROC_MAP_STYLE_ICE      =  0,
    CROC_MAP_STYLE_WATER    =  1,
    CROC_MAP_STYLE_CASTLE   =  2,
    CROC_MAP_STYLE_DESERT   =  3,
    CROC_MAP_STYLE_CAVE     =  4,
    CROC_MAP_STYLE_WOOD     =  5,
    CROC_MAP_STYLE_DUNGEON  =  6,
    CROC_MAP_STYLE_BOSS1    =  7,
    CROC_MAP_STYLE_BOSS2    =  8,
    CROC_MAP_STYLE_BOSS3    =  9,
    CROC_MAP_STYLE_BOSS4    = 10,
    CROC_MAP_STYLE_BOSS5    = 11,
    CROC_MAP_STYLE_BOSS6    = 12,
    CROC_MAP_STYLE_BOSS7    = 13,
    CROC_MAP_STYLE_BOSS8    = 14,
    CROC_MAP_STYLE_BOSS9    = 15,
    /* 16 */
    CROC_MAP_STYLE_ISLAND1  = 17,
    CROC_MAP_STYLE_ISLAND2  = 18,
    CROC_MAP_STYLE_ISLAND3  = 19,
    CROC_MAP_STYLE_ISLAND4  = 20,
    CROC_MAP_STYLE_ISLAND5  = 21,
    CROC_MAP_STYLE_ICESLIDE = 22,
    CROC_MAP_STYLE_BOULDER  = 23,
    CROC_MAP_STYLE_ICECAVE  = 24,
    CROC_MAP_STYLE_MAX
} CrocMapStyle;

typedef enum CrocMapEffect {
    CROC_MAP_EFFECT_NONE                = 0,
    CROC_MAP_EFFECT_SNOW                = 1,
    /* 2 */
    CROC_MAP_EFFECT_FOG                 = 3,
    /* 4 */
    /* 5 */
    /* 6 */
    CROC_MAP_EFFECT_TORCH               = 7,
    CROC_MAP_EFFECT_TORCH_LIGHTNING     = 8,
    CROC_MAP_EFFECT_TORCH_BIG           = 9,
    CROC_MAP_EFFECT_TORCH_BIG_LIGHTNING = 10,
    CROC_MAP_EFFECT_DARK                = 11,
    CROC_MAP_EFFECT_SNOW_SMALL          = 12,
    /* 13 */
    CROC_MAP_EFFECT_SNOW_ISLAND         = 14,
    CROC_MAP_EFFECT_TORCH_GIANT         = 15,
    CROC_MAP_EFFECT_MAX
} CrocMapEffect;

/*
 * Ambient Tracks.
 * - Am unsure about WHAL. Track 17 was missing in any of the maps
 *   but there's an unused AMBIWHAL.BIN in the PSX releases, so I'm
 *   guessing track 17 == WHAL.
 * - There's also an AMBIATLA.BIN in the PSX releases, roughly the same
 *   size as the other ambient BIN files, but it's in the sound wad in
 *   the PC release. It's not used in any of the maps either, so I'm leaving
 *   it out.
 */
typedef enum CrocMapAmbience
{
    CROC_MAP_AMBI_NONE =  0, /* None        */
    CROC_MAP_AMBI_BIRD =  1, /* Bird        */
    CROC_MAP_AMBI_DRIP =  2, /* Drip        */
    CROC_MAP_AMBI_NGHT =  3, /* Night       */
    CROC_MAP_AMBI_WATR =  4, /* Water       */
    CROC_MAP_AMBI_CRWD =  5, /* Crowd       */
    CROC_MAP_AMBI_LAVA =  6, /* Lava        */
    CROC_MAP_AMBI_TUNL =  7, /* Tunnel      */
    CROC_MAP_AMBI_ICE1 =  8, /* Ice 1       */
    CROC_MAP_AMBI_TOMB =  9, /* Tomb        */
    CROC_MAP_AMBI_DES1 = 10, /* Desert 1    */
    CROC_MAP_AMBI_DCAV = 11, /* Desert Cave */
    CROC_MAP_AMBI_SLDE = 12, /* Ice Slide   */
    CROC_MAP_AMBI_CAST = 13, /* Castle      */
    CROC_MAP_AMBI_DNGN = 14, /* Dungeon     */
    CROC_MAP_AMBI_ICE2 = 15, /* Ice 2       */
    CROC_MAP_AMBI_CLWD = 16, /* Cloud World */
    CROC_MAP_AMBI_WHAL = 17, /* Whale       */
    CROC_MAP_AMBI_MOAT = 18, /* Moat        */
    CROC_MAP_AMBI_DUSK = 19, /* Dusk        */
    CROC_MAP_AMBI_WIND = 20, /* Wind        */
    CROC_MAP_AMBI_CICA = 21, /* Cicada      */
    CROC_MAP_AMBI_DANT = 22, /* Dantini     */
    CROC_MAP_AMBI_MAX
} CrocMapAmbience;

typedef enum CrocMapDoorFlags
{
    /* A normal door, through to the next level. */
    CROC_MAP_DOOR_NORMAL    = (1 << 0),
    /* A floor-door, where Croc comes up from. */
    CROC_MAP_DOOR_FLOOR	    = (1 << 1),
    /* A ceiling door, when Croc falls from a magical place. */
    CROC_MAP_DOOR_CEILING   = (1 << 2),
    /*
     * The starting "door", where Croc spawns.
     * Skipped when searching for the next door.
     */
    CROC_MAP_DOOR_START     = (1 << 3),
    /*
     * A "final" door, returns Croc to the island.
     * Only used in some hidden places in maps, presumably
     * for debug purposes.
     */
    CROC_MAP_DOOR_FINAL     = (1 << 4),
    CROC_MAP_DOOR_COUNT     = 5
} CrocMapDoorFlags;

typedef struct CrocColour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t pad;
} CrocColour;

typedef struct CrocVector
{
    croc_x0412_t x;
    croc_x0412_t y;
    croc_x0412_t z;
    croc_x0412_t pad;
} CrocVector;

/*
 * Not sure why these are called tracks. "Tiles"
 * would have been better. From "TR%02d_TRK.MOD"
 */
typedef struct CrocMapTrack
{
    uint16_t     x;
    uint16_t     y;
    uint16_t     z;
    croc_x0412_t yr;
    /* Submesh index. */
    uint16_t     index;
    /* These seem to do nothing. */
    uint16_t     reserved0;
    uint16_t     reserved1;
} CrocMapTrack;

typedef struct CrocMapWaypoint
{
    croc_x2012_t x;
    croc_x2012_t y;
    croc_x2012_t z;
    uint32_t     var;
} CrocMapWaypoint;

typedef struct CrocMapStrat
{
    uint32_t        params[CROC_MAP_MAX_PARAMS];
    croc_x2012_t    x;
    croc_x2012_t    y;
    croc_x2012_t    z;
    croc_x0412_t    xr;
    croc_x0412_t    yr;
    croc_x0412_t    zr;
    char            name[CROC_MAP_STRING_LEN + 1];
    uint16_t        num_waypoints;
    CrocMapWaypoint *waypoint;
} CrocMapStrat;

typedef struct CrocMapDoor
{
    uint16_t         x;
    uint16_t         y;
    uint16_t         z;
    uint16_t         level;
    uint16_t         sublevel;
    CrocMapDoorFlags flags;
    /*
     * Door ID, links doors together across maps.
     * - See the top/bottom doors in MP014_00 and MP014_00
     */
    uint16_t         id;
} CrocMapDoor;

typedef struct CrocMapPointLight
{
    croc_x1616_t x;
    croc_x1616_t y;
    croc_x1616_t z;
    CrocColour   colour;
    croc_x1616_t fade_from;
    croc_x1616_t fade_to;
} CrocMapPointLight;

typedef struct CrocMapDirectLight
{
    CrocVector vector;
    CrocColour colour;
} CrocMapDirectLight;

typedef struct CrocMap {
    /* Format of the map. One of the CROC_MAPFMT* constants. */
    CrocMapFormat       _format;
    /* Version number. Known are 12, 20, 21. */
    uint16_t            _version;
    uint16_t            _level;
    uint16_t            _sublevel;
    uint32_t            _checksum;

    char                path[CROC_MAP_STRING_LEN + 1];
    char                name[CROC_MAP_STRING_LEN + 1];

    uint16_t            width;
    uint16_t            height;
    uint16_t            depth;
    CrocMapStyle        style;
    /*
     * I _think_ these are flags of some sort.
     * Further investigation is required.
     */
    uint32_t            flags;
    /* CD track. In the PC version, an index into the remap table. */
    uint32_t            cd_track;
    /* Skydome number. */
    uint32_t            background;
    CrocMapEffect       effect;
    /* Croc's wait animation, one of the CROCWA%d.ANI files. */
    uint32_t            wait;
    CrocMapAmbience     ambience;

    uint16_t            num_tracks;
    croc_x0412_t        start_rotation;

    CrocMapTrack		*track;

    uint16_t           num_strats;
    CrocMapStrat       *strat;

    uint16_t           num_doors;
    CrocMapDoor        *door;

    /* Point lights, only in > simple lighting mode. */
    uint16_t           num_point_lights;
    CrocMapPointLight  *point_light;

    /* 3 fixed directional lights. */
    CrocMapDirectLight direct_light[CROC_MAP_MAX_DIRECT_LIGHTS];

    /* Global ambient colour. */
    CrocColour         ambient_colour;
} CrocMap;

#endif /* _LIBCROC_MAPDEF_H */
