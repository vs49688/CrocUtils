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
#include <errno.h>
#include <stdlib.h>
#include <vsclib.h>
#include "map.h"


void croc_map_init(CrocMap *map)
{
    if(map == NULL)
        return;

    memset(map, 0, sizeof(CrocMap));

    /* Init with some basic defaults. */
    map->_version   = 21;
    map->_format    = CROC_MAP_FMT_NORMAL;
    map->_level     = 0;
    map->_sublevel  = 0;
    map->_checksum  = 0;

    strncpy(map->name, "Default New Map", CROC_MAP_STRING_LEN);
    map->name[CROC_MAP_STRING_LEN] = '\0';

    map->width      = 16;
    map->height     = 16;
    map->depth      = 32;
    map->style      = CROC_MAP_STYLE_ICE;

    map->ambient_colour.r   = 64;
    map->ambient_colour.g   = 64;
    map->ambient_colour.b   = 64;
    map->ambient_colour.pad =  0;
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
