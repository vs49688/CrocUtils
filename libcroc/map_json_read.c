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
#include <string.h>
#include <cJSON.h>
#include <vsclib.h>
#include <libcroc/fixed.h>
#include <libcroc/map.h>

static cJSON *get_type(const cJSON *j, const char *name, cJSON_bool(*proc)(const cJSON *))
{
    cJSON *tmp;

    if((tmp = cJSON_GetObjectItemCaseSensitive(j, name)) == NULL)
        return NULL;

    if(!proc(tmp))
        return NULL;

    return tmp;
}

static int read_string(const cJSON *j, const char *name, char *buf, size_t len)
{
    cJSON *tmp;

    if((tmp = get_type(j, name, cJSON_IsString)) == NULL)
        return -1;

    strncpy(buf, tmp->valuestring, len);
    return 0;
}

static int read_uint16(const cJSON *j, const char *name, uint16_t *val)
{
    cJSON *tmp;

    if((tmp = get_type(j, name, cJSON_IsNumber)) == NULL)
        return -1;

    *val = (uint16_t)tmp->valuedouble;
    return 0;
}

static int read_uint32(const cJSON *j, const char *name, uint32_t *val)
{
    cJSON *tmp;

    if((tmp = get_type(j, name, cJSON_IsNumber)) == NULL)
        return -1;

    *val = (uint32_t)tmp->valuedouble;
    return 0;
}

static int read_float(const cJSON *j, const char *name, float *val)
{
    cJSON *tmp;

    if((tmp = get_type(j, name, cJSON_IsNumber)) == NULL)
        return -1;

    *val = (float)tmp->valuedouble;
    return 0;
}

static int read_x0412(const cJSON *j, const char *name, croc_x0412_t *val)
{
    float f;

    if(read_float(j, name, &f) < 0)
        return -1;

    *val = croc_float_to_x0412(f);
    return 0;
}

static int read_x1616(const cJSON *j, const char *name, croc_x1616_t *val)
{
    float f;

    if(read_float(j, name, &f) < 0)
        return -1;

    *val = croc_float_to_x1616(f);
    return 0;
}

static int read_style(const cJSON *j, const char *name, CrocMapStyle *style)
{
    cJSON *tmp;

    /* Handle if this is a number. */
    if((tmp = get_type(j, name, cJSON_IsNumber)) != NULL) {
        uint16_t val = (uint16_t)tmp->valuedouble;
        if(val >= CROC_MAP_STYLE_MAX)
            return -1;

        *style = (CrocMapStyle)val;
        return 0;
    }

    if((tmp = get_type(j, name, cJSON_IsString)) == NULL)
        return -1;

    int i;
    for(i = 0; i < CROC_MAP_STYLE_MAX; ++i) {
        if(CrocMapStyleStrings[i] == NULL)
            continue;

        if(strcmp(CrocMapStyleStrings[i], tmp->valuestring) == 0)
            break;
    }

    if(i == CROC_MAP_STYLE_MAX)
        return -1;

    *style = (CrocMapStyle)i;
    return 0;
}

static int read_effect(const cJSON *j, const char *name, CrocMapEffect *effect)
{
    cJSON *tmp;

    /* Handle if this is a number. */
    if((tmp = get_type(j, name, cJSON_IsNumber)) != NULL) {
        uint16_t val = (uint16_t)tmp->valuedouble;
        if(val >= CROC_MAP_EFFECT_MAX)
            return -1;

        *effect = (CrocMapEffect)val;
        return 0;
    }

    if((tmp = get_type(j, name, cJSON_IsString)) == NULL)
        return -1;

    int i;
    for(i = 0; i < CROC_MAP_EFFECT_MAX; ++i) {
        if(CrocMapEffectStrings[i] == NULL)
            continue;

        if(strcmp(CrocMapEffectStrings[i], tmp->valuestring) == 0)
            break;
    }

    if(i == CROC_MAP_EFFECT_MAX)
        return -1;

    *effect = (CrocMapEffect)i;
    return 0;
}


static int read_format(const cJSON *j, const char *name, CrocMapFormat *format)
{
    cJSON *tmp;

    /* Handle if this is a number. */
    if((tmp = get_type(j, name, cJSON_IsNumber)) != NULL) {
        uint16_t val = (uint16_t)tmp->valuedouble;
        if(val >= CROC_MAP_FMT_INVALID)
            return -1;

        *format = (CrocMapFormat)val;
        return 0;
    }

    if((tmp = get_type(j, name, cJSON_IsString)) == NULL)
        return -1;

    int i;
    for(i = 0; i < CROC_MAP_FMT_INVALID; ++i) {
        if(CrocMapFmtStrings[i] == NULL)
            continue;

        if(strcmp(CrocMapFmtStrings[i], tmp->valuestring) == 0)
            break;
    }

    if(i == CROC_MAP_FMT_INVALID)
        return -1;

    *format = (CrocMapFormat)i;
    return 0;
}

static int read_ambience(const cJSON *j, const char *name, CrocMapAmbience *ambience)
{
    cJSON *tmp;

    /* Handle if this is a number. */
    if((tmp = get_type(j, name, cJSON_IsNumber)) != NULL) {
        uint16_t val = (uint16_t)tmp->valuedouble;
        if(val >= CROC_MAP_AMBI_MAX)
            return -1;

        *ambience = (CrocMapAmbience)val;
        return 0;
    }

    if((tmp = get_type(j, name, cJSON_IsString)) == NULL)
        return -1;

    int i;
    for(i = 0; i < CROC_MAP_AMBI_MAX; ++i) {
        if(CrocMapAmbienceStrings[i] == NULL)
            continue;

        if(strcmp(CrocMapAmbienceStrings[i], tmp->valuestring) == 0)
            break;
    }

    if(i == CROC_MAP_AMBI_MAX)
        return -1;

    *ambience = (CrocMapAmbience)i;
    return 0;
}

static int read_float_array(const cJSON *object, const char *name, float *v, size_t n)
{
    cJSON *tmp, *val;
    size_t i;

    if((tmp = get_type(object, name, cJSON_IsArray)) == NULL)
        return -1;

    if(cJSON_GetArraySize(tmp) != n)
        return -1;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        if(!cJSON_IsNumber(val))
            return -1;

        v[i++] = (float)val->valuedouble;
    }

    return 0;
}

static int read_uint16_vec3(const cJSON *object, const char *name, uint16_t *x, uint16_t *y, uint16_t *z)
{
    float v[3];
    if(read_float_array(object, name, v, 3) < 0)
        return -1;

    *x = (uint16_t)v[0];
    *y = (uint16_t)v[1];
    *z = (uint16_t)v[2];
    return 0;
}

static int read_x0412_vec3(const cJSON *object, const char *name, croc_x0412_t *x, croc_x0412_t *y, croc_x0412_t *z)
{
    float v[3];
    if(read_float_array(object, name, v, 3) < 0)
        return -1;

    *x = croc_float_to_x0412(v[0]);
    *y = croc_float_to_x0412(v[1]);
    *z = croc_float_to_x0412(v[2]);;
    return 0;
}

static int read_x1616_vec3(const cJSON *object, const char *name, croc_x1616_t *x, croc_x1616_t *y, croc_x1616_t *z)
{
    float v[3];
    if(read_float_array(object, name, v, 3) < 0)
        return -1;

    *x = croc_float_to_x1616(v[0]);
    *y = croc_float_to_x1616(v[1]);
    *z = croc_float_to_x1616(v[2]);;
    return 0;
}

static int read_x2012_vec3(const cJSON *object, const char *name, croc_x2012_t *x, croc_x2012_t *y, croc_x2012_t *z)
{
    float v[3];
    if(read_float_array(object, name, v, 3) < 0)
        return -1;

    *x = croc_float_to_x2012(v[0]);
    *y = croc_float_to_x2012(v[1]);
    *z = croc_float_to_x2012(v[2]);;
    return 0;
}

static int read_uint8_vec3(const cJSON *object, const char *name, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float v[3];
    if(read_float_array(object, name, v, 3) < 0)
        return -1;

    *r = (uint8_t)v[0];
    *g = (uint8_t)v[1];
    *b = (uint8_t)v[2];
    return -0;
}

static int read_params_array(const cJSON *object, const char *name, uint32_t *v)
{
    float f[8];
    if(read_float_array(object, name, f, 8) < 0)
        return -1;

    for(int i = 0; i < 8; ++i)
        v[i] = (uint32_t)f[i];

    return 0;
}

static CrocMapTrack *read_tracks(const cJSON *j, const char *name, uint16_t *count)
{
    cJSON *tmp, *val;
    size_t size, i;
    CrocMapTrack *tracks;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return NULL;

    size = (size_t)cJSON_GetArraySize(tmp);
    if(size >= UINT16_MAX)
        return NULL;

    if((tracks = vsc_calloc(size, sizeof(CrocMapTrack))) == NULL)
        return NULL;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapTrack *t = tracks + i;

        if(read_uint16_vec3(val, "position", &t->x, &t->y, &t->z) < 0)
            goto fail;

        if(read_x0412(val, "yr", &t->yr) < 0)
            goto fail;

        if(read_uint16(val, "index", &t->index) < 0)
            goto fail;

        t->reserved0 = 0;
        t->reserved1 = 0;

        ++i;
    }

    *count = (uint16_t)size;
    return tracks;

fail:
    vsc_free(tracks);
    return NULL;
}

static CrocMapWaypoint *read_waypoints(const cJSON *j, const char *name, uint16_t *count)
{
    cJSON *tmp, *val;
    size_t size, i;
    CrocMapWaypoint *waypoints;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return NULL;

    size = (size_t)cJSON_GetArraySize(tmp);
    if(size >= UINT16_MAX)
        return NULL;

    if((waypoints = vsc_calloc(size, sizeof(CrocMapWaypoint))) == NULL)
        return NULL;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapWaypoint *wp = waypoints + i;

        if(read_x2012_vec3(val, "position", &wp->x, &wp->y, &wp->z) < 0)
            goto fail;

        if(read_uint32(val, "var", &wp->var) < 0)
            goto fail;
        ++i;
    }

    *count = (uint16_t)size;
    return waypoints;

fail:
    vsc_free(waypoints);
    return NULL;
}

static CrocMapStrat *read_strats(const cJSON *j, const char *name, uint16_t *count)
{
    cJSON *tmp, *val;
    size_t size, i;
    CrocMapStrat *strats;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return NULL;

    size = (size_t)cJSON_GetArraySize(tmp);
    if(size >= UINT16_MAX)
        return NULL;

    if((strats = vsc_calloc(size, sizeof(CrocMapStrat))) == NULL)
        return NULL;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapStrat *s = strats + i;

        if(read_params_array(val, "params", s->params) < 0)
            goto fail;

        if(read_x2012_vec3(val, "position", &s->x, &s->y, &s->z) < 0)
            goto fail;

        if(read_x0412_vec3(val, "rotation", &s->xr, &s->yr, &s->zr) < 0)
            goto fail;

        if(read_string(val, "name", s->name, CROC_MAP_STRING_LEN) < 0)
            goto fail;

        s->name[CROC_MAP_STRING_LEN] = '\0';

        if((s->waypoint = read_waypoints(val, "waypoints", &s->num_waypoints)) == NULL)
            goto fail;

        ++i;
    }

    *count = (uint16_t)size;

    return strats;

fail:

    for(i = 0; i < size; ++i) {
        if(strats[i].waypoint != NULL)
            vsc_free(strats[i].waypoint);

    }
    vsc_free(strats);
    return NULL;
}

static int read_door_flags(const cJSON *j, const char *name, CrocMapDoorFlags *flags)
{
    cJSON *tmp, *val;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return -1;

    *flags = 0;
    cJSON_ArrayForEach(val, tmp) {
        if(!cJSON_IsString(val))
            return -1;

        if(strcmp(CrocMapDoorFlagStrings[CROC_MAP_DOOR_NORMAL], val->valuestring) == 0)
            *flags |= CROC_MAP_DOOR_NORMAL;
        else if(strcmp(CrocMapDoorFlagStrings[CROC_MAP_DOOR_FLOOR], val->valuestring) == 0)
            *flags |= CROC_MAP_DOOR_FLOOR;
        else if(strcmp(CrocMapDoorFlagStrings[CROC_MAP_DOOR_CEILING], val->valuestring) == 0)
            *flags |= CROC_MAP_DOOR_CEILING;
        else if(strcmp(CrocMapDoorFlagStrings[CROC_MAP_DOOR_START], val->valuestring) == 0)
            *flags |= CROC_MAP_DOOR_START;
        else if(strcmp(CrocMapDoorFlagStrings[CROC_MAP_DOOR_FINAL], val->valuestring) == 0)
            *flags |= CROC_MAP_DOOR_FINAL;
        else
            return -1;
    }
    return 0;
}

static CrocMapDoor *read_doors(const cJSON *j, const char *name, uint16_t *count)
{
    cJSON *tmp, *val;
    size_t size, i;
    CrocMapDoor *doors;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return NULL;

    size = (size_t)cJSON_GetArraySize(tmp);
    if(size >= UINT16_MAX)
        return NULL;

    if((doors = vsc_calloc(size, sizeof(CrocMapDoor))) == NULL)
        return NULL;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapDoor *d = doors + i;

        if(read_uint16_vec3(val, "position", &d->x, &d->y, &d->z) < 0)
            goto fail;

        if(read_uint16(val, "level", &d->level) < 0)
            goto fail;

        if(read_uint16(val, "sublevel", &d->sublevel) < 0)
            goto fail;

        if(read_door_flags(val, "flags", &d->flags) < 0)
            goto fail;

        if(read_uint16(val, "id", &d->id) < 0)
            goto fail;

        ++i;
    }

    *count = (uint16_t)size;
    return doors;

fail:
    vsc_free(doors);
    return NULL;
}


static CrocMapPointLight *read_point_lights(const cJSON *j, const char *name, uint16_t *count)
{
    cJSON *tmp, *val;
    size_t size, i;
    CrocMapPointLight *lights;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return NULL;

    size = (size_t)cJSON_GetArraySize(tmp);
    if(size >= UINT16_MAX)
        return NULL;

    if((lights = vsc_calloc(size, sizeof(CrocMapPointLight))) == NULL)
        return NULL;

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapPointLight *l = lights + i;

        if(read_x1616_vec3(val, "position", &l->x, &l->y, &l->z) < 0)
            goto fail;

        if(read_uint8_vec3(val, "colour", &l->colour.r, &l->colour.g, &l->colour.b) < 0)
            goto fail;

        l->colour.pad = 0;

        if(read_x1616(val, "fade_from", &l->fade_from) < 0)
            goto fail;

        if(read_x1616(val, "fade_to", &l->fade_to) < 0)
            goto fail;

        ++i;
    }

    *count = (uint16_t)size;
    return lights;

fail:
    vsc_free(lights);
    return NULL;
}

static int read_direct_lights(const cJSON *j, const char *name, CrocMapDirectLight *lights, int count)
{
    cJSON *tmp, *val;
    int i;

    if((tmp = get_type(j, name, cJSON_IsArray)) == NULL)
        return -1;

    if(cJSON_GetArraySize(tmp) != count)
        return -1;

    memset(lights, 0, sizeof(CrocMapDirectLight) * count);

    i = 0;
    cJSON_ArrayForEach(val, tmp) {
        CrocMapDirectLight *l = lights + i;

        if(read_x0412_vec3(val, "position", &l->vector.x, &l->vector.y, &l->vector.z) < 0)
            return -1;

        if(read_uint8_vec3(val, "colour", &l->colour.r, &l->colour.g, &l->colour.b) < 0)
            return -1;

        ++i;
    }

    return 0;
}

static int read_meta(const cJSON *j, const char *name, CrocMap *map)
{
    cJSON *tmp;

    /* This field is optional */
    if((tmp = get_type(j, name, cJSON_IsObject)) == NULL)
        return 0;

    if(read_uint16(tmp, "version", &map->_version) < 0)
        return -1;

    if(read_format(tmp, "format", &map->_format) < 0)
        return -1;

    if(read_uint16(tmp, "level", &map->_level) < 0)
        return -1;

    if(read_uint16(tmp, "sublevel", &map->_sublevel) < 0)
        return -1;

    if(read_uint32(tmp, "checksum", &map->_checksum) < 0)
        return -1;

    return 0;
}

CrocMap *croc_map_read_json(const cJSON *j, CrocMap *map)
{
    croc_map_free(map);

    if(read_meta(j, "_meta", map) < 0)
        goto fail;

    if(read_string(j, "path", map->path, CROC_MAP_STRING_LEN) < 0)
        goto fail;

    map->path[CROC_MAP_STRING_LEN] = '\0';

    if(read_string(j, "name", map->name, CROC_MAP_STRING_LEN) < 0)
        goto fail;

    map->name[CROC_MAP_STRING_LEN] = '\0';

    if(read_uint16(j, "width", &map->width) < 0)
        goto fail;

    if(read_uint16(j, "height", &map->height) < 0)
        goto fail;

    if(read_uint16(j, "depth", &map->depth) < 0)
        goto fail;

    if(read_style(j, "style", &map->style) < 0)
        goto fail;

    if(read_uint32(j, "flags", &map->flags) < 0)
        goto fail;

    if(read_uint32(j, "cd_track", &map->cd_track) < 0)
        goto fail;

    if(read_uint32(j, "background", &map->background) < 0)
        goto fail;

    if(read_effect(j, "effect", &map->effect) < 0)
        goto fail;

    if(read_uint32(j, "wait", &map->wait) < 0)
        goto fail;

    if(read_ambience(j, "ambience", &map->ambience) < 0)
        goto fail;

    if(read_x0412(j, "start_rotation", &map->start_rotation) < 0)
        goto fail;

    if((map->track = read_tracks(j, "tracks", &map->num_tracks)) == NULL)
        goto fail;

    if((map->strat = read_strats(j, "strats", &map->num_strats)) == NULL)
        goto fail;

    if((map->door = read_doors(j, "doors", &map->num_doors)) == NULL)
        goto fail;

    if((map->point_light = read_point_lights(j, "point_lights", &map->num_point_lights)) == NULL)
        goto fail;

    if(read_direct_lights(j, "direct_lights", map->direct_light, CROC_MAP_MAX_DIRECT_LIGHTS) < 0)
        goto fail;

    if(read_uint8_vec3(j, "ambient_colour", &map->ambient_colour.r, &map->ambient_colour.g, &map->ambient_colour.b) < 0)
        goto fail;

    map->ambient_colour.pad = 0;

    return map;

fail:
    croc_map_free(map);
    return NULL;
}
