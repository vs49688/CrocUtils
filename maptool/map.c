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

/* On-disk sizes of some of the structures. */
#define CROC_MAP_TRACK_SIZE			14
#define CROC_MAP_TRACK_SIZE_PREV21	10
#define CROC_MAP_WAYPOINT_SIZE		16
#define CROC_MAP_POINT_LIGHT_SIZE	24
#define CROC_MAP_DIRECT_LIGHT_SIZE	12
#define CROC_MAP_COLOUR_SIZE		 4
#define CROC_MAP_BASE_STRAT_SIZE	50
#define CROC_MAP_DOOR_SIZE			14

/* Read an 8-bit length-prefixed string. */
static int read_mapstring(FILE *f, char *buf, uint8_t *len)
{
	uint8_t l;

	l = vsc_fread_uint8(f);
	if(feof(f) || ferror(f)) {
		errno = EIO;
		return -1;
	}

	if(l == 0) {
		*buf = '\0';
		return l;
	}

	if(fread(buf, l, 1, f) != 1) {
		errno = EIO;
		return -1;
	}

	buf[l] = '\0';

	if(len != NULL)
		*len = l;

	errno = 0;
	return l;
}

static CrocMapTrack *read_tracks(FILE *f, uint16_t version, uint16_t num_tracks)
{
	CrocMapTrack *tracks = NULL;
	size_t size = version >= 21 ? CROC_MAP_TRACK_SIZE : CROC_MAP_TRACK_SIZE_PREV21;

	if((tracks = malloc(sizeof(CrocMapTrack) * num_tracks)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for(int i = 0; i < num_tracks; ++i) {
		CrocMapTrack *t = tracks + i;
		uint8_t tmp[CROC_MAP_TRACK_SIZE];
		if(fread(&tmp, size, 1, f) != 1) {
			free(tracks);
			errno = EIO;
			return NULL;
		}

		t->x               = vsc_read_leu16(tmp +  0);
		t->y               = vsc_read_leu16(tmp +  2);
		t->z               = vsc_read_leu16(tmp +  4);
		t->yr.v            = vsc_read_le16 (tmp +  6);
		t->index           = vsc_read_leu16(tmp +  8);

		if(version >= 21) {
			t->reserved0   = vsc_read_leu16(tmp + 10);
			t->reserved1   = vsc_read_leu16(tmp + 12);
		} else {
			t->reserved0   = 0;
			t->reserved1   = 0;
		}
	}

	errno = 0;
	return tracks;
}


static void croc_mapstrat_free(CrocMapStrat *s, size_t count)
{
	if(s == NULL)
		return;

	for(size_t i = 0; i < count; ++i) {
		CrocMapStrat *ms = s + i;

		if(ms->waypoint != NULL)
			free(ms->waypoint);
	}

	free(s);
}

static int read_strat_v12(FILE *f, CrocMapStrat *s)
{
	uint16_t nparam;

	nparam = vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		return -1;
	}

	if(nparam > CROC_MAP_MAX_PARAMS) {
		errno = ERANGE;
		return -1;
	}

	if(fread(s->params, sizeof(uint32_t), nparam, f) != nparam) {
		errno = EIO;
		return -1;
	}

	for(int p = nparam; p < CROC_MAP_MAX_PARAMS; ++p)
		s->params[p] = 0;

	s->x.v  = vsc_fread_leu32(f);
	s->y.v  = vsc_fread_leu32(f);
	s->z.v  = vsc_fread_leu32(f);
	s->xr.v = vsc_fread_leu16(f);
	s->yr.v = vsc_fread_leu16(f);
	s->zr.v = vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		return -1;
	}

	return 0;
}

static int read_strat(FILE *f, CrocMapStrat *s)
{
	if(fread(s->params, sizeof(uint32_t), CROC_MAP_MAX_PARAMS, f) != CROC_MAP_MAX_PARAMS) {
		errno = EIO;
		return -1;
	}

	s->x.v  = vsc_fread_le32(f);
	s->y.v  = vsc_fread_le32(f);
	s->z.v  = vsc_fread_le32(f);
	s->xr.v = vsc_fread_le16(f);
	s->yr.v = vsc_fread_le16(f);
	s->zr.v = vsc_fread_le16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		return -1;
	}

	return 0;
}

static CrocMapStrat *read_strats(FILE *f, uint16_t format, uint16_t num_strats)
{
	CrocMapStrat *strats = NULL;
	int err = 0;

	if((strats = malloc(sizeof(CrocMapStrat) * num_strats)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	memset(strats, 0, sizeof(CrocMapStrat) * num_strats);

	for(int i = 0; i < num_strats; ++i)
	{
		CrocMapStrat *s = strats + i;
		if(format == CROC_MAP_FMT_NORMAL)
			err = read_strat(f, s);
		else
			err = read_strat_v12(f, s);

		if(err < 0) {
			err = errno;
			goto fail;
		}

		for(int p = 0; p < CROC_MAP_MAX_PARAMS; ++p)
			s->params[p] = vsc_leu32_to_native(s->params[p]);

		/* Strats must have names. */
		if(read_mapstring(f, s->name, NULL) <= 0)
			goto fail;

		s->num_waypoints = vsc_fread_leu16(f);
		if(feof(f) || ferror(f)) {
			err = ENOMEM;
			goto fail;
		}

		if((s->waypoint = malloc(sizeof(CrocMapWaypoint) * s->num_waypoints)) == NULL) {
			err = ENOMEM;
			goto fail;
		}

		for(int w = 0; +w < s->num_waypoints; ++w) {
			CrocMapWaypoint *wp = s->waypoint + w;
			uint8_t buf[CROC_MAP_WAYPOINT_SIZE];

			if(fread(buf, sizeof(buf), 1, f) != 1) {
				err = EIO;
				goto fail;
			}

			wp->x.v	= vsc_read_le32(buf +  0);
			wp->y.v	= vsc_read_le32(buf +  4);
			wp->z.v	= vsc_read_le32(buf +  8);
			wp->var	= vsc_read_leu32(buf + 12);
		}
	}

	errno = 0;
	return strats;

fail:
	croc_mapstrat_free(strats, num_strats);
	errno = err;
	return NULL;
}

static CrocMapDoor *read_doors(FILE *f, uint16_t num_doors)
{
	CrocMapDoor *doors = NULL;
	uint8_t buf[CROC_MAP_DOOR_SIZE];

	if((doors = malloc(sizeof(CrocMapDoor) * num_doors)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for(int i = 0; i < num_doors; ++i) {
		CrocMapDoor *d = doors + i;

		if(fread(buf, CROC_MAP_DOOR_SIZE, 1, f) != 1) {
			free(doors);
			errno = EIO;
			return NULL;
		}

		d->x		= vsc_read_leu16(buf +  0);
		d->y		= vsc_read_leu16(buf +  2);
		d->z		= vsc_read_leu16(buf +  4);
		d->level	= vsc_read_leu16(buf +  6);
		d->sublevel	= vsc_read_leu16(buf +  8);
		d->flags	= vsc_read_leu16(buf + 10);
		d->id		= vsc_read_leu16(buf + 12);
	}

	errno = 0;
	return doors;
}


static CrocColour *read_colour(void *p, CrocColour *c)
{
	uint8_t *d = p;
	c->r   = vsc_read_uint8(d + 0);
	c->g   = vsc_read_uint8(d + 1);
	c->b   = vsc_read_uint8(d + 2);
	c->pad = vsc_read_uint8(d + 3);
	return c;
}

static CrocVector *read_vector(void *p, CrocVector *v)
{
	uint8_t *d = p;
	v->x.v   = vsc_read_le16(d + 0);
	v->y.v   = vsc_read_le16(d + 2);
	v->z.v   = vsc_read_le16(d + 4);
	v->pad.v = vsc_read_le16(d + 6);
	return v;
}

static CrocColour *fread_colour(FILE *f, CrocColour *c)
{
	uint8_t buf[CROC_MAP_COLOUR_SIZE];
	if(fread(buf, CROC_MAP_COLOUR_SIZE, 1, f) != 1) {
		errno = EIO;
		return NULL;
	}

	return read_colour(buf, c);
}

static CrocMapPointLight *read_point(FILE *f, uint16_t num)
{
	CrocMapPointLight *point = NULL;
	uint8_t buf[CROC_MAP_POINT_LIGHT_SIZE];

	if((point = malloc(sizeof(CrocMapPointLight) * num)) == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	for(int i = 0; i < num; ++i) {
		CrocMapPointLight *l = point + i;

		if(fread(buf, CROC_MAP_POINT_LIGHT_SIZE, 1, f) != 1) {
			free(point);
			errno = EIO;
			return NULL;
		}

		l->x.v = vsc_read_le32(buf + 0);
		l->y.v = vsc_read_le32(buf + 4);
		l->z.v = vsc_read_le32(buf + 8);

		read_colour(buf + 12, &l->colour);

		l->fade_from.v = vsc_read_le32(buf + 16);
		l->fade_to.v   = vsc_read_le32(buf + 20);
	}

	return point;
}

static CrocMapDirectLight *read_direct(FILE *f, CrocMap *map)
{
	uint8_t buf[CROC_MAP_DIRECT_LIGHT_SIZE * CROC_MAP_MAX_DIRECT_LIGHTS];

	if(fread(buf, CROC_MAP_DIRECT_LIGHT_SIZE, CROC_MAP_MAX_DIRECT_LIGHTS, f) != CROC_MAP_MAX_DIRECT_LIGHTS) {
		errno = EIO;
		return NULL;
	}

	uint8_t *tmp = buf;
	for(int i = 0; i < CROC_MAP_MAX_DIRECT_LIGHTS; ++i, tmp += CROC_MAP_DIRECT_LIGHT_SIZE) {
		CrocMapDirectLight *l = map->direct_light + i;
		read_vector(tmp + 0, &l->vector);
		read_colour(tmp + 8, &l->colour);
	}

	return map->direct_light;
}

void croc_map_free(CrocMap *map)
{
	if(map == NULL)
		return;

	if(map->point_light)
		free(map->point_light);

	if(map->door)
		free(map->door);

	croc_mapstrat_free(map->strat, map->num_strats);

	if(map->track)
		free(map->track);

	croc_map_init(map);
}

int croc_map_read(FILE *f, CrocMap *map)
{
	uint16_t tmp;
	int err = 0;
	long stratstart;

	croc_map_free(map);

	memset(map, 0, sizeof(CrocMap));

	tmp = vsc_fread_leu16(f);

	if(tmp == 12)
	{
		map->_format  = CROC_MAP_FMT_V12_OLD;
		map->_version = 12;
		vsc_fread_uint8(f); /* pad */
	}
	else
	{
		/* Skip the rest of the size. */
		vsc_fread_leu16(f);

		map->_version = vsc_fread_leu16(f);
		if(map->_version == 12 || map->_version == 20 || map->_version == 21)
			map->_format = CROC_MAP_FMT_NORMAL;
		else
			map->_format = CROC_MAP_FMT_INVALID;

		vsc_fread_leu16(f); /* pad */
	}

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if(map->_format == CROC_MAP_FMT_INVALID) {
		errno = EINVAL;
		return -1;
	}

	if(read_mapstring(f, map->path, NULL) < 0)
		goto fail;

	vsc_fread_leu16(f); /* Total waypoint count, not used for us. */

	map->width			= vsc_fread_leu16(f);
	map->height			= vsc_fread_leu16(f);
	map->depth			= vsc_fread_leu16(f);
	map->style			= vsc_fread_leu16(f);
	map->flags			= vsc_fread_leu32(f);
	map->cd_track		= vsc_fread_leu32(f);
	map->background		= vsc_fread_leu32(f);
	map->effect			= vsc_fread_leu32(f);
	map->wait			= vsc_fread_leu32(f);
	map->ambience		= vsc_fread_leu32(f);

	vsc_fread_leu16(f); /* pad */

	map->num_tracks		= vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if(map->style >= CROC_MAP_STYLE_MAX) {
		errno = EIO;
		goto fail;
	}

	if(map->effect >= CROC_MAP_EFFECT_MAX) {
		errno = EIO;
		goto fail;
	}

	if(map->ambience >= CROC_MAP_AMBI_MAX) {
		errno = EIO;
		goto fail;
	}

	if(read_mapstring(f, map->name, NULL) < 0)
		goto fail;

	map->start_rotation.v = vsc_fread_le16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if((map->track = read_tracks(f, map->_version, map->num_tracks)) == NULL)
		goto fail;

	map->num_strats = vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if((stratstart = ftell(f)) < 0)
		goto fail;

	if((map->strat = read_strats(f, map->_format, map->num_strats)) == NULL) {
		/*
		 * If reading fails, and we're a CROC_MAP_FMT_NORMAL, version 12 map,
		 * we might actually be a CROC_MAP_FMT_V12.
		 *
		 * This is nasty, but there's no other way to check.
		 */
		if(map->_version != 12 || map->_format != CROC_MAP_FMT_NORMAL || ferror(f))
			goto fail;

		if(fseek(f, stratstart, SEEK_SET) < 0)
			goto fail;

		map->_format = CROC_MAP_FMT_V12;
		if((map->strat = read_strats(f, map->_format, map->num_strats)) == NULL)
			goto fail;
	}

	map->num_doors = vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if((map->door = read_doors(f, map->num_doors)) == NULL)
		goto fail;

	map->num_point_lights = vsc_fread_leu16(f);

	if(feof(f) || ferror(f)) {
		errno = EIO;
		goto fail;
	}

	if((map->point_light = read_point(f, map->num_point_lights)) == NULL)
		goto fail;

	if(read_direct(f, map) == NULL)
		goto fail;

	if(fread_colour(f, &map->ambient_colour) == NULL) {
		/* Some v12 maps don't have this. */
		if(map->_version != 12 && !feof(f))
			goto fail;

		map->ambient_colour.r   = 64;
		map->ambient_colour.g   = 64;
		map->ambient_colour.b   = 64;
		map->ambient_colour.pad = 0;

		map->_format = CROC_MAP_FMT_V12_OLD_NOAMBIENT;
	}

	/* Checksum is literally a sum of all the bytes in the file -.- */
	if((map->_format == CROC_MAP_FMT_NORMAL || map->_format == CROC_MAP_FMT_V12) && !feof(f))
		map->_checksum = vsc_fread_leu32(f);
	else
		map->_checksum = 0;

	/* NB: Ignore errors, we don't care if the checksum fails. */

	return 0;

fail:
	err = errno;
	croc_map_free(map);
	errno = err;
	return -1;
}

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
