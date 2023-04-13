/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

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
    unsigned long _level, _sublevel = 0;
    const char *start = croc_util_get_filename(path);
    char *endptr;

    if(strlen(start) != 12)
        return -1;

    if((start[ 0] != 'm' && start[ 0] != 'M') ||
       (start[ 1] != 'p' && start[ 1] != 'P') ||
       (start[ 5] != '_')                     ||
       (start[ 8] != '.')                     ||
       (start[ 9] != 'm' && start[ 9] != 'M') ||
       (start[10] != 'a' && start[10] != 'A') ||
       (start[11] != 'p' && start[11] != 'P')) {
        return -1;
    }

    errno = 0;
    _level = strtoul(start + 2, &endptr, 10);
    if(errno != 0 || *endptr != '_')
        return -1;

    errno = 0;
    _sublevel = strtoul(start + 6, &endptr, 10);
    if(errno != 0 || *endptr != '.')
        return -1;

    if(_level > CROC_MAP_MAX_LEVEL || _sublevel > CROC_MAP_MAX_SUBLEVEL)
        return -1;

    *level    = (uint16_t)_level;
    *sublevel = (uint16_t)_sublevel;

    return 0;
}
