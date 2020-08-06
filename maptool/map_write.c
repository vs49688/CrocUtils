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

#include <stdio.h>
#include <vsclib.h>
#include <errno.h>
#include <stdlib.h>
#include "map.h"

static int write_mapstring(FILE *f, const char *s)
{
    size_t len = s ? strlen(s) : 0;
    if(len > 255)
        len = 255;

    if(vsc_fwrite_uint8(f, (uint8_t)len) != 1) {
        errno = EIO;
        return -1;
    }

    if(len == 0)
        return 0;

    if(fwrite(s, len, 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    return 0;
}

static int write_tracks(FILE *f, const CrocMapTrack *tracks, uint16_t num)
{
    uint8_t buf[CROC_MAP_TRACK_SIZE];

    for(int i = 0; i < num; ++i) {
        const CrocMapTrack *t = tracks + i;

        vsc_write_leu16(buf +  0, t->x);
        vsc_write_leu16(buf +  2, t->y);
        vsc_write_leu16(buf +  4, t->z);
        vsc_write_le16 (buf +  6, t->yr.v);
        vsc_write_leu16(buf +  8, t->index);
        vsc_write_leu16(buf + 10, t->reserved0);
        vsc_write_leu16(buf + 12, t->reserved1);

        if(fwrite(buf, CROC_MAP_TRACK_SIZE, 1, f) != 1) {
            errno = EIO;
            return -1;
        }
    }

    return 0;
}

static int write_strats(FILE *f, const CrocMapStrat *strats, uint16_t num)
{
    uint8_t buf[CROC_MAX(CROC_MAP_BASE_STRAT_SIZE, CROC_MAP_WAYPOINT_SIZE)];

    for(int i = 0; i < num; ++i) {
        const CrocMapStrat *s = strats + i;

        for(int p = 0; p < CROC_MAP_MAX_PARAMS; ++p) {
            vsc_write_leu32(buf + 4 * p, s->params[p]);
        }

        vsc_write_le32(buf + 32, s->x.v);
        vsc_write_le32(buf + 36, s->y.v);
        vsc_write_le32(buf + 40, s->z.v);
        vsc_write_le16(buf + 44, s->xr.v);
        vsc_write_le16(buf + 46, s->yr.v);
        vsc_write_le16(buf + 48, s->zr.v);

        if(fwrite(buf, CROC_MAP_BASE_STRAT_SIZE, 1, f) != 1) {
            errno = EIO;
            return -1;
        }

        if(write_mapstring(f, s->name) < 0)
            return -1;

        if(vsc_fwrite_leu16(f, s->num_waypoints) != 1) {
            errno = EIO;
            return -1;
        }

        for(int w = 0; w < s->num_waypoints; ++w) {
            CrocMapWaypoint *wp = s->waypoint + w;

            vsc_write_le32( buf +  0, wp->x.v);
            vsc_write_le32( buf +  4, wp->y.v);
            vsc_write_le32( buf +  8, wp->z.v);
            vsc_write_leu32(buf + 12, wp->var);

            if(fwrite(buf, CROC_MAP_WAYPOINT_SIZE, 1, f) != 1) {
                errno = EIO;
                return -1;
            }
        }
    }

    return 0;
}

static int write_doors(FILE *f, const CrocMapDoor *doors, uint16_t num)
{
    uint8_t buf[CROC_MAP_DOOR_SIZE];

    for(int i = 0; i < num; ++i) {
        const CrocMapDoor *d = doors + i;

        /* TODO: See if these aren't actually fixed-point. */
        vsc_write_leu16(buf +  0, d->x);
        vsc_write_leu16(buf +  2, d->y);
        vsc_write_leu16(buf +  4, d->z);
        vsc_write_leu16(buf +  6, d->level);
        vsc_write_leu16(buf +  8, d->sublevel);
        vsc_write_leu16(buf + 10, d->flags);
        vsc_write_leu16(buf + 12, d->id);

        if(fwrite(buf, CROC_MAP_DOOR_SIZE, 1, f) != 1) {
            errno = EIO;
            return -1;
        }
    }

    return 0;
}

static void write_colour(void *p, const CrocColour *c)
{
    uint8_t *d = p;
    vsc_write_uint8(d + 0, c->r);
    vsc_write_uint8(d + 1, c->g);
    vsc_write_uint8(d + 2, c->b);
    vsc_write_uint8(d + 3, c->pad);
}

static size_t fwrite_colour(FILE *f, const CrocColour *c)
{
    uint8_t buf[CROC_MAP_COLOUR_SIZE];
    write_colour(buf, c);

    return fwrite(buf, CROC_MAP_COLOUR_SIZE, 1, f);
}

static void write_vector(void *p, const CrocVector *v)
{
    uint8_t *d = p;
    vsc_write_le16(d + 0, v->x.v);
    vsc_write_le16(d + 2, v->y.v);
    vsc_write_le16(d + 4, v->z.v);
    vsc_write_le16(d + 6, v->pad.v);
}

static int write_point(FILE *f, const CrocMapPointLight *point, uint16_t num)
{
    uint8_t buf[CROC_MAP_POINT_LIGHT_SIZE];

    for(int i = 0; i < num; ++i) {
        const CrocMapPointLight *l = point + i;

        vsc_write_le32(buf +  0, l->x.v);
        vsc_write_le32(buf +  4, l->y.v);
        vsc_write_le32(buf +  8, l->z.v);

        write_colour(buf + 12, &l->colour);

        vsc_write_le32(buf + 16, l->fade_from.v);
        vsc_write_le32(buf + 20, l->fade_to.v);

        if(fwrite(buf, CROC_MAP_POINT_LIGHT_SIZE, 1, f) != 1) {
            errno = EIO;
            return -1;
        }

    }
    return 0;
}

static int write_direct(FILE *f, const CrocMap *map)
{
    uint8_t buf[CROC_MAP_DIRECT_LIGHT_SIZE * CROC_MAP_MAX_DIRECT_LIGHTS];

    uint8_t *tmp = buf;
    for(int i = 0; i < CROC_MAP_MAX_DIRECT_LIGHTS; ++i, tmp += CROC_MAP_DIRECT_LIGHT_SIZE) {
        const CrocMapDirectLight *l = map->direct_light + i;
        write_vector(tmp + 0, &l->vector);
        write_colour(tmp + 8, &l->colour);
    }

    if(fwrite(buf, sizeof(buf), 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    return 0;
}

int croc_map_write(FILE *f, const CrocMap *map)
{
    unsigned int wpc;
    long startoffset, endoffset;

    /* Get the total waypoint count. */
    wpc = 0;
    for(int i = 0; i < map->num_strats; ++i) {
        wpc += map->strat[i].num_waypoints;
    }

    if(wpc > UINT16_MAX) {
        errno = ERANGE;
        return -1;
    }

    if((startoffset = ftell(f)) < 0)
        return -1;

    /* Size, we fix this later. */
    if(vsc_fwrite_leu32(f, 0) != 1)
        goto iofail;

    /* Version */
    if(vsc_fwrite_leu16(f, 21) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, 0) != 1)
        goto iofail;

    if(write_mapstring(f, map->path) < 0)
        return -1;

    if(vsc_fwrite_leu16(f, (uint16_t)wpc) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, map->width) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, map->height) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, map->depth) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, map->style) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->flags) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->cd_track) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->background) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->effect) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->wait) != 1)
        goto iofail;

    if(vsc_fwrite_leu32(f, map->ambience) != 1)
        goto iofail;

    if(vsc_fwrite_leu16(f, 0) != 1) /* pad */
        goto iofail;


    if(vsc_fwrite_leu16(f, map->num_tracks) != 1)
        goto iofail;

    if(write_mapstring(f, map->name) < 0)
        return -1;

    if(vsc_fwrite_le16(f, map->start_rotation.v) != 1)
        goto iofail;

    if(write_tracks(f, map->track, map->num_tracks) < 0)
        return -1;

    if(vsc_fwrite_leu16(f, map->num_strats) < 0)
        goto iofail;

    if(write_strats(f, map->strat, map->num_strats) < 0)
        return -1;

    if(vsc_fwrite_leu16(f, map->num_doors) != 1)
        goto iofail;

    if(write_doors(f, map->door, map->num_doors) < 0)
        return -1;

    if(vsc_fwrite_leu16(f, map->num_point_lights) != 1)
        goto iofail;

    if(write_point(f, map->point_light, map->num_point_lights) < 0)
        return -1;

    if(write_direct(f, map) < 0)
        return -1;

    if(fwrite_colour(f, &map->ambient_colour) != 1)
        goto iofail;

    /* Write the size field. */

    if((endoffset = ftell(f)) < 0) /* Sets errno */
        return -1;

    if(fseek(f, startoffset, SEEK_SET) < 0)
        return -1;

    if(vsc_fwrite_leu32(f, (uint32_t)((endoffset - startoffset) - 4)) != 1)
        goto iofail;

    /*
     * FIXME: Actually calculate the checksum. The game doesn't check it anyway.
     *  Will need to rip this all out once vsclib has its IO abstraction. Then
     *  the checksum can be calculated on-the-fly.
     */
    if(fseek(f, 0, SEEK_END) < 0)
        return -1;

    if(vsc_fwrite_leu32(f, map->_checksum) != 1)
        goto iofail;

    return 0;

iofail:
    errno = EIO;
    return -1;
}