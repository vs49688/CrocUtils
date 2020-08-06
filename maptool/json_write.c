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
#include "map.h"
#include "cJSON.h"

static cJSON *add_float_array(cJSON *object, const char *name, float *v, int n)
{
    cJSON *j;

    if((j = cJSON_CreateFloatArray(v, n)) == NULL)
        return NULL;

    cJSON_AddItemToObject(object, name, j);

    return j;
}

static cJSON *add_x1616_vec3(cJSON *object, const char *name, croc_x1616_t x, croc_x1616_t y, croc_x1616_t z)
{
    float v[3] = {
        croc_x1616_to_float(x),
        croc_x1616_to_float(y),
        croc_x1616_to_float(z)
    };

    return add_float_array(object, name, v, 3);
}

static cJSON *add_x2012_vec3(cJSON *object, const char *name, croc_x2012_t x, croc_x2012_t y, croc_x2012_t z)
{
    float v[3] = {
        croc_x2012_to_float(x),
        croc_x2012_to_float(y),
        croc_x2012_to_float(z)
    };

    return add_float_array(object, name, v, 3);
}

static cJSON *add_x0412_vec3(cJSON *object, const char *name, croc_x0412_t x, croc_x0412_t y, croc_x0412_t z)
{
    float v[3] = {
        croc_x0412_to_float(x),
        croc_x0412_to_float(y),
        croc_x0412_to_float(z)
    };

    return add_float_array(object, name, v, 3);
}

static cJSON *add_uint8_vec3(cJSON *object, const char *name, uint8_t r, uint8_t g, uint8_t b)
{
    float v[3] = { r, g, b };
    return add_float_array(object, name, v, 3);
}

static cJSON *add_uint16_vec3(cJSON *object, const char *name, uint16_t x, uint16_t y, uint16_t z)
{
    float v[3] = { x, y, z };
    return add_float_array(object, name, v, 3);
}

static cJSON *add_track_to_array(cJSON *array, const CrocMapTrack *t)
{
    cJSON *jt = NULL;

    assert(array->type == cJSON_Array);

    if((jt = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, jt))
        return NULL;

    if(add_uint16_vec3(jt, "position", t->x, t->y, t->z) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jt, "yr", croc_x0412_to_float(t->yr)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jt, "index", t->index) == NULL)
        return NULL;

    return jt;
}

static cJSON *add_waypoint_to_array(cJSON *array, const CrocMapWaypoint *wp)
{
    cJSON *j = NULL;

    assert(array->type == cJSON_Array);

    if((j = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, j))
        return NULL;

    if(add_x2012_vec3(j, "position", wp->x, wp->y, wp->z) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "var", wp->var) == NULL)
        return NULL;

    return j;
}

static cJSON *add_strat_to_array(cJSON *array, const CrocMapStrat *s)
{
    cJSON *j = NULL, *jp = NULL, *jw = NULL;

    assert(array->type == cJSON_Array);

    if((j = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, j))
        return NULL;

    if((jp = cJSON_AddArrayToObject(j, "params")) == NULL)
        return NULL;

    for(int i = 0; i < CROC_MAP_MAX_PARAMS; ++i) {
        cJSON *jn = cJSON_CreateNumber(s->params[i]);
        if(jn == NULL)
            return NULL;

        if(!cJSON_AddItemToArray(jp, jn))
            return NULL;
    }

    if(add_x2012_vec3(j, "position", s->x, s->y, s->z) == NULL)
        return NULL;

    if(add_x0412_vec3(j, "rotation", s->xr, s->yr, s->zr) == NULL)
        return NULL;

    if(cJSON_AddStringToObject(j, "name", s->name) == NULL)
        return NULL;


    if((jw = cJSON_AddArrayToObject(j, "waypoints")) == NULL)
        return NULL;

    for(int i = 0; i < s->num_waypoints; ++i) {
        if(add_waypoint_to_array(jw, s->waypoint + i) == NULL)
            return NULL;
    }

    return j;
}

static cJSON *add_door_flags(cJSON *door, const char *name, CrocMapDoorFlags flags)
{
    cJSON *j = NULL;
    const char *strings[CROC_MAP_DOOR_COUNT] = {0};
    int count = 0;

    if(flags & CROC_MAP_DOOR_NORMAL)
        strings[count++] = CrocMapDoorFlagStrings[CROC_MAP_DOOR_NORMAL];

    if(flags & CROC_MAP_DOOR_FLOOR)
        strings[count++] = CrocMapDoorFlagStrings[CROC_MAP_DOOR_FLOOR];

    if(flags & CROC_MAP_DOOR_CEILING)
        strings[count++] = CrocMapDoorFlagStrings[CROC_MAP_DOOR_CEILING];

    if(flags & CROC_MAP_DOOR_START)
        strings[count++] = CrocMapDoorFlagStrings[CROC_MAP_DOOR_START];

    if(flags & CROC_MAP_DOOR_FINAL)
        strings[count++] = CrocMapDoorFlagStrings[CROC_MAP_DOOR_FINAL];

    if((j = cJSON_CreateStringArray(strings, count)) == NULL)
        return NULL;

    if(!cJSON_AddItemToObject(door, name, j)) {
        cJSON_Delete(j);
        return NULL;
    }

    return j;
}

static cJSON *add_door_to_array(cJSON *array, const CrocMapDoor *door)
{
    cJSON *jd = NULL;

    assert(array->type == cJSON_Array);

    if((jd = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, jd))
        return NULL;

    if(add_uint16_vec3(jd, "position", door->x, door->y, door->z) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jd, "level", door->level) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jd, "sublevel", door->sublevel) == NULL)
        return NULL;

    if(add_door_flags(jd, "flags", door->flags) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jd, "id", door->id) == NULL)
        return NULL;

    return jd;
}

static cJSON *add_point_light_to_array(cJSON *array, const CrocMapPointLight *l)
{
    cJSON *jl = NULL;

    assert(array->type == cJSON_Array);

    if((jl = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, jl))
        return NULL;

    if(add_x1616_vec3(jl, "position", l->x, l->y, l->z) == NULL)
        return NULL;

    if(add_uint8_vec3(jl, "colour", l->colour.r, l->colour.g, l->colour.b) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jl, "fade_from", croc_x1616_to_float(l->fade_from)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jl, "fade_to", croc_x1616_to_float(l->fade_to)) == NULL)
        return NULL;

    return jl;
}

static cJSON *add_direct_light_to_array(cJSON *array, const CrocMapDirectLight *l)
{
    cJSON *jl = NULL;

    assert(array->type == cJSON_Array);

    if((jl = cJSON_CreateObject()) == NULL)
        return NULL;

    if(!cJSON_AddItemToArray(array, jl))
        return NULL;

    if(add_x0412_vec3(jl, "position", l->vector.x, l->vector.y, l->vector.z) == NULL)
        return NULL;

    if(add_uint8_vec3(jl, "colour", l->colour.r, l->colour.g, l->colour.b) == NULL)
        return NULL;

    return jl;
}


static cJSON *add_meta_to_object(cJSON *jmap, const char *name, const CrocMap *map)
{
    cJSON *jmeta;

    assert(jmap->type == cJSON_Object);

    if(map->_format >= CROC_MAP_FMT_INVALID)
        return NULL;

    if((jmeta = cJSON_AddObjectToObject(jmap, name)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmeta, "version", map->_version) == NULL)
        return NULL;

    if(cJSON_AddStringToObject(jmeta, "format", CrocMapFmtStrings[map->_format]) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmeta, "level", map->_level) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmeta, "sublevel", map->_sublevel) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmeta, "checksum", map->_checksum) == NULL)
        return NULL;

    return jmeta;
}

cJSON *croc_map_write_json(const CrocMap *map)
{
    cJSON *jmap = NULL, *jtmp = NULL;

    if(map == NULL)
        return NULL;

    if((jmap = cJSON_CreateObject()) == NULL)
        return NULL;

    if(add_meta_to_object(jmap, "_meta", map) == NULL)
        goto fail;

    if(cJSON_AddStringToObject(jmap, "path", map->path) == NULL)
        goto fail;

    if(cJSON_AddStringToObject(jmap, "name", map->name) == NULL)
        goto fail;

    if(cJSON_AddNumberToObject(jmap, "width", map->width) == NULL)
        goto fail;

    if(cJSON_AddNumberToObject(jmap, "height", map->height) == NULL)
        goto fail;

    if(cJSON_AddNumberToObject(jmap, "depth", map->depth) == NULL)
        goto fail;

    if(map->style >= CROC_MAP_STYLE_MAX)
        return NULL;

    if(CrocMapStyleStrings[map->style] == NULL)
        jtmp = cJSON_AddNumberToObject(jmap, "style", map->style);
    else
        jtmp = cJSON_AddStringToObject(jmap, "style", CrocMapStyleStrings[map->style]);

    if(jtmp == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmap, "flags", map->flags) == NULL)
        goto fail;

    if(cJSON_AddNumberToObject(jmap, "cd_track", map->cd_track) == NULL)
        goto fail;

    if(cJSON_AddNumberToObject(jmap, "background", map->background) == NULL)
        goto fail;

    if(map->effect >= CROC_MAP_EFFECT_MAX)
        return NULL;

    if(CrocMapEffectStrings[map->effect] == NULL)
        jtmp = cJSON_AddNumberToObject(jmap, "effect", map->effect);
    else
        jtmp = cJSON_AddStringToObject(jmap, "effect", CrocMapEffectStrings[map->effect]);

    if(jtmp == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmap, "wait", map->wait) == NULL)
        goto fail;

    if(map->ambience >= CROC_MAP_AMBI_MAX)
        return NULL;

    if(CrocMapAmbienceStrings[map->ambience] == NULL)
        jtmp = cJSON_AddNumberToObject(jmap, "ambience", map->ambience);
    else
        jtmp = cJSON_AddStringToObject(jmap, "ambience", CrocMapAmbienceStrings[map->ambience]);

    if(jtmp == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(jmap, "start_rotation", croc_x0412_to_float(map->start_rotation)) == NULL)
        goto fail;

    /* Tracks */
    if((jtmp = cJSON_AddArrayToObject(jmap, "tracks")) == NULL)
        goto fail;

    for(int i = 0; i < map->num_tracks; ++i) {
        if(add_track_to_array(jtmp, map->track + i) == NULL)
            goto fail;
    }

    /* Strats */
    if((jtmp = cJSON_AddArrayToObject(jmap, "strats")) == NULL)
        goto fail;

    for(int i = 0; i < map->num_strats; ++i) {
        if(add_strat_to_array(jtmp, map->strat + i) == NULL)
            goto fail;
    }

    /* Doors */
    if((jtmp = cJSON_AddArrayToObject(jmap, "doors")) == NULL)
        goto fail;

    for(int i = 0; i < map->num_doors; ++i) {
        if(add_door_to_array(jtmp, map->door + i) == NULL)
            goto fail;
    }

    if((jtmp = cJSON_AddArrayToObject(jmap, "point_lights")) == NULL)
        goto fail;

    for(int i = 0; i < map->num_point_lights; ++i) {
        if(add_point_light_to_array(jtmp, map->point_light + i) == NULL)
            goto fail;
    }

    if((jtmp = cJSON_AddArrayToObject(jmap, "direct_lights")) == NULL)
        goto fail;

    for(int i = 0; i < CROC_MAP_MAX_DIRECT_LIGHTS; ++i) {
        if(add_direct_light_to_array(jtmp, map->direct_light + i) == NULL)
            goto fail;
    }

    if(add_uint8_vec3(jmap, "ambient_colour", map->ambient_colour.r,
       map->ambient_colour.g, map->ambient_colour.b) == NULL)
        goto fail;

    return jmap;

fail:
    cJSON_free(jmap);
    return NULL;
}
