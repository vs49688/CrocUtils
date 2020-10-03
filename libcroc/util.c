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
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <libcroc/util.h>
#include <libcroc/mapdef.h>

const char *croc_util_get_filename(const char *path)
{
	const char *start = strrchr(path, '/');
	if(start == NULL)
		start = strrchr(path, '\\');

	if(start == NULL)
		return path;
	else
		return start + 1;
}

int croc_extract_level_info(const char *path, uint16_t *level, uint16_t *sublevel)
{
    uint32_t _level, _sublevel;
    int pos;
    char c = '\0';
    const char *start = croc_util_get_filename(path);

    if(sscanf(start, "%*[mM]%*[pP]%03u_%02u.%*[mM]%*[aA]%[pP]%n", &_level, &_sublevel, &c, &pos) != 3)
        return -1;

    if(c != 'p' && c != 'P')
        return -1;

    if(start[pos] != '\0')
        return -1;

    if(_level > CROC_MAP_MAX_LEVEL || _sublevel > CROC_MAP_MAX_SUBLEVEL)
        return -1;

    *level    = (uint16_t)_level;
    *sublevel = (uint16_t)_sublevel;

    return 0;
}
