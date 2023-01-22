#include "defs.h"

static void write_mtl(FILE *fp, br_material *mat)
{
    MaterialUser *user = mat->user;
    float         ka, kd, ks, power;

    ka    = BrScalarToFloat(BrUFractionToScalar(mat->ka));
    kd    = BrScalarToFloat(BrUFractionToScalar(mat->kd));
    ks    = BrScalarToFloat(BrUFractionToScalar(mat->ks));
    power = BrScalarToFloat(mat->power);

    if(power == 1710618.0f)
        power = log2f(power);

    fprintf(fp, "##\n");
    fprintf(fp, "# %s\n", mat->identifier);
    fprintf(fp, "# Filename: %s\n", user->filename);
    if(user->is_styled) {
        fprintf(fp, "# Style: %s\n", CrocMapStyleStrings[user->style]);
    } else {
        fprintf(fp, "# Style: none\n");
    }
    fprintf(fp, "##\n");

    fprintf(fp, "newmtl %s\n", mat->identifier);
    fprintf(fp, "    Ka %f %f %f\n", ka, ka, ka);
    fprintf(fp, "    Ks %f %f %f\n", ks, ks, ks);
    fprintf(fp, "    Ns %f\n", power);
    fprintf(fp, "    Tr %f\n", (float)mat->opacity / 255.0f);

    if(mat->colour_map != NULL) {
        /* Has a texture, use it, ignore colour. */
        fprintf(fp, "    map_Kd textures/%s.tga", mat->colour_map->identifier);

        if(mat->mode & (BR_MATM_MAP_WIDTH_LIMIT_CLAMP | BR_MATM_MAP_HEIGHT_LIMIT_CLAMP))
            fprintf(fp, " -clamp on");
        else
            fprintf(fp, " -clamp off");

        fprintf(fp, "\n");

        fprintf(fp, "    Kd %f %f %f\n", kd, kd, kd);
        //        fprintf(fp, "    Kd 1.0 0.0 0.0\n");
    } else {
        /* No texture, use colour. */
        float r = (float)BR_RED(mat->colour) / 255.0f;
        float g = (float)BR_GRN(mat->colour) / 255.0f;
        float b = (float)BR_BLU(mat->colour) / 255.0f;
        fprintf(fp, "    Kd %f %f %f\n", r * kd, g * kd, b * kd);
    }

    if(mat->ka == 0 && mat->ks == 0) {
        fprintf(fp, "    illum 0\n");
    } else if(mat->ks == 0) {
        fprintf(fp, "    illum 1\n");
    } else {
        fprintf(fp, "    illum 2\n");
    }

    fprintf(fp, "\n");
}

#include <cJSON.h>

static cJSON *add_colour(cJSON *parent, const char *name, br_colour col)
{
    cJSON *j;

    if((j = cJSON_AddObjectToObject(parent, name)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "red", BR_RED(col) / 255.0f) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "green", BR_GRN(col) / 255.0f) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "blue", BR_BLU(col) / 255.0f) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "alpha", BR_ALPHA(col) / 255.0f) == NULL)
        return NULL;

    return j;
}

static cJSON *add_pm_ref(cJSON *parent, const char *name, br_pixelmap *pm)
{
    if(pm == NULL)
        return cJSON_AddNullToObject(parent, name);
    else
        return cJSON_AddStringToObject(parent, name, pm->identifier);
}

STATIC const struct {
    const char *name;
    int         value;
} MaterialFlagNames[] = {
    {"light",               BR_MATF_LIGHT              },
    {"prelit",              BR_MATF_PRELIT             },
    {"smooth",              BR_MATF_SMOOTH             },
    {"environment",         BR_MATF_ENVIRONMENT_I      },
    {"environment_local",   BR_MATF_ENVIRONMENT_L      },
    {"perspective",         BR_MATF_PERSPECTIVE        },
    {"decal",               BR_MATF_DECAL              },
    {"always_visible",      BR_MATF_ALWAYS_VISIBLE     },
    {"two_sided",           BR_MATF_TWO_SIDED          },
    {"force_front",         BR_MATF_FORCE_FRONT        },
    {"force_back",          BR_MATF_FORCE_BACK         },
    {"dither",              BR_MATF_DITHER             },
    {"map_antialiasing",    BR_MATF_MAP_ANTIALIASING   },
    {"map_interpolation",   BR_MATF_MAP_INTERPOLATION  },
    {"mip_interpolation",   BR_MATF_MIP_INTERPOLATION  },
    {"subdivide",           BR_MATF_SUBDIVIDE          },
    {"fog_local",           BR_MATF_FOG_LOCAL          },
    {"quad_mapping",        BR_MATF_QUAD_MAPPING       },
    {"inhibit_depth_write", BR_MATF_INHIBIT_DEPTH_WRITE},
};

STATIC const struct {
    const char *name;
    int         value;
} DepthTestNames[] = {
    {"gt", BR_MATM_DEPTH_TEST_GT},
    {"ge", BR_MATM_DEPTH_TEST_GE},
    {"eq", BR_MATM_DEPTH_TEST_EQ},
    {"ne", BR_MATM_DEPTH_TEST_NE},
    {"le", BR_MATM_DEPTH_TEST_LE},
    {"lt", BR_MATM_DEPTH_TEST_LT},
    {"nv", BR_MATM_DEPTH_TEST_NV},
    {"al", BR_MATM_DEPTH_TEST_AL},
};

STATIC const struct {
    const char *name;
    int         value;
} BlendModeNames[] = {
    {"standard",      BR_MATM_BLEND_MODE_STANDARD     },
    {"summed",        BR_MATM_BLEND_MODE_SUMMED       },
    {"dimmed",        BR_MATM_BLEND_MODE_DIMMED       },
    {"premultiplied", BR_MATM_BLEND_MODE_PREMULTIPLIED},
};

STATIC const struct {
    const char *name;
    int         value;
} WidthLimitNames[] = {
    {"wrap",   BR_MATM_MAP_WIDTH_LIMIT_WRAP  },
    {"clamp",  BR_MATM_MAP_WIDTH_LIMIT_CLAMP },
    {"mirror", BR_MATM_MAP_WIDTH_LIMIT_MIRROR},
};

STATIC const struct {
    const char *name;
    int         value;
} HeightLimitNames[] = {
    {"wrap",   BR_MATM_MAP_HEIGHT_LIMIT_WRAP  },
    {"clamp",  BR_MATM_MAP_HEIGHT_LIMIT_CLAMP },
    {"mirror", BR_MATM_MAP_HEIGHT_LIMIT_MIRROR},
};

static cJSON *add_mat_flags(cJSON *parent, const char *name, br_uint_32 flags)
{
    cJSON *j;

    if((j = cJSON_AddArrayToObject(parent, name)) == NULL)
        return NULL;

    for(size_t i = 0; i < BR_ASIZE(MaterialFlagNames); ++i) {
        if(flags & MaterialFlagNames[i].value) {
            cJSON *js;

            if((js = cJSON_CreateString(MaterialFlagNames[i].name)) == NULL)
                return NULL;

            cJSON_AddItemToArray(j, js);
        }
    }

    return j;
}

static cJSON *add_map_transform(cJSON *parent, const char *name, br_matrix23 *map_transform)
{
    cJSON *j, *j2;

    if((j = cJSON_CreateArray()) == NULL)
        return NULL;

    cJSON_AddItemToObject(parent, name, j);

    if((j2 = cJSON_CreateFloatArray(map_transform->m[0], 2)) == NULL)
        return NULL;

    cJSON_AddItemToArray(j, j2);

    if((j2 = cJSON_CreateFloatArray(map_transform->m[1], 2)) == NULL)
        return NULL;

    cJSON_AddItemToArray(j, j2);

    if((j2 = cJSON_CreateFloatArray(map_transform->m[2], 2)) == NULL)
        return NULL;

    cJSON_AddItemToArray(j, j2);

    return j;
}

static cJSON *add_depth_test(cJSON *parent, const char *name, br_uint_16 mode)
{
    for(size_t i = 0; i < BR_ASIZE(DepthTestNames); ++i) {
        if((mode & BR_MATM_DEPTH_TEST_MASK) == DepthTestNames[i].value) {
            return cJSON_AddStringToObject(parent, name, DepthTestNames[i].name);
        }
    }

    return NULL;
}

static cJSON *add_blend_mode(cJSON *parent, const char *name, br_uint_16 mode)
{
    for(size_t i = 0; i < BR_ASIZE(BlendModeNames); ++i) {
        if((mode & BR_MATM_BLEND_MODE_MASK) == BlendModeNames[i].value) {
            return cJSON_AddStringToObject(parent, name, BlendModeNames[i].name);
        }
    }

    return NULL;
}

static cJSON *add_mal_width_limit(cJSON *parent, const char *name, br_uint_16 mode)
{
    for(size_t i = 0; i < BR_ASIZE(WidthLimitNames); ++i) {
        if((mode & BR_MATM_MAP_WIDTH_LIMIT_MASK) == WidthLimitNames[i].value) {
            return cJSON_AddStringToObject(parent, name, WidthLimitNames[i].name);
        }
    }

    return NULL;
}

static cJSON *add_mal_height_limit(cJSON *parent, const char *name, br_uint_16 mode)
{
    for(size_t i = 0; i < BR_ASIZE(HeightLimitNames); ++i) {
        if((mode & BR_MATM_MAP_HEIGHT_LIMIT_MASK) == HeightLimitNames[i].value) {
            return cJSON_AddStringToObject(parent, name, HeightLimitNames[i].name);
        }
    }

    return NULL;
}

static cJSON *create_material_json(br_material *mat)
{
    cJSON *j;
    float  power = BrScalarToFloat(mat->power);

    if(power == 1710618.0f)
        power = log2f(power);

    if((j = cJSON_CreateObject()) == NULL)
        return NULL;

    if(cJSON_AddStringToObject(j, "identifier", mat->identifier) == NULL)
        return NULL;

    if(add_colour(j, "colour", mat->colour) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "opacity", (float)mat->opacity / 255.0f) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "ka", BrUFractionToScalar(mat->ka)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "kd", BrUFractionToScalar(mat->kd)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "ks", BrUFractionToScalar(mat->ks)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "power", power) == NULL)
        return NULL;

    if(add_mat_flags(j, "flags", mat->flags) == NULL)
        return NULL;

    if(add_map_transform(j, "map_transform", &mat->map_transform) == NULL)
        return NULL;

    if(add_depth_test(j, "depth_test", mat->mode) == NULL)
        return NULL;

    if(add_blend_mode(j, "blend_mode", mat->mode) == NULL)
        return NULL;

    if(add_mal_width_limit(j, "map_width_limit", mat->mode) == NULL)
        return NULL;

    if(add_mal_height_limit(j, "map_height_limit", mat->mode) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "index_base", mat->index_base) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "index_range", mat->index_range) == NULL)
        return NULL;

    if(add_pm_ref(j, "colour_map", mat->colour_map) == NULL)
        return NULL;

    if(add_pm_ref(j, "screendoor", mat->screendoor) == NULL)
        return NULL;

    if(add_pm_ref(j, "index_shade", mat->index_shade) == NULL)
        return NULL;

    if(add_pm_ref(j, "index_blend", mat->index_blend) == NULL)
        return NULL;

    if(add_pm_ref(j, "index_fog", mat->index_fog) == NULL)
        return NULL;

    // TODO: extra surf
    // TODO: extra prim

    if(cJSON_AddNumberToObject(j, "fog_min", BrScalarToFloat(mat->fog_min)) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "fog_max", BrScalarToFloat(mat->fog_max)) == NULL)
        return NULL;

    if(add_colour(j, "fog_colour", mat->fog_colour) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "subdivide_tolerance", mat->subdivide_tolerance) == NULL)
        return NULL;

    if(cJSON_AddNumberToObject(j, "depth_bias", mat->depth_bias) == NULL)
        return NULL;

    return j;
}

static int mtlwriter(const void *key, void *value, vsc_hash_t hash, void *user)
{
    br_material *mat = value;
    FILE        *fp  = user;

    write_mtl(fp, mat);
    return 0;
}

static int mtlwriter_json(const void *key, void *value, vsc_hash_t hash, void *user)
{
    br_material *mat    = value;
    cJSON       *parent = user, *j;

    if((j = create_material_json(mat)) == NULL)
        return VSC_ERROR(ENOMEM);

    if(!cJSON_AddItemToObject(parent, key, j))
        return VSC_ERROR(EINVAL);

    return 0;
}

int crocconvert_material_register_styled(CrocConvert *cc, CrocMapStyle style, const char *identifier, br_material *mat)
{
    if(cc == NULL || identifier == NULL)
        return VSC_ERROR(EINVAL);

    if(mat == NULL)
        return 0;

    if(style == CROC_MAP_STYLE_MAX)
        style = cc->current_style;

    return vsc_hashmap_insert(cc->mat_styled + style, identifier, mat);
}

int crocconvert_material_register(CrocConvert *cc, const char *identifier, br_material *mat)
{
    if(cc == NULL || identifier == NULL)
        return VSC_ERROR(EINVAL);

    if(mat == NULL)
        return 0;

    return vsc_hashmap_insert(&cc->mat_unstyled, identifier, mat);
}

br_material *crocconvert_material_find_styled(CrocConvert *cc, CrocMapStyle style, const char *identifier)
{
    if(cc == NULL || identifier == NULL)
        return NULL;

    if(style == CROC_MAP_STYLE_MAX)
        style = cc->current_style;

    return vsc_hashmap_find(cc->mat_styled + style, identifier);
}

br_material *crocconvert_material_find_unstyled(CrocConvert *cc, const char *identifier)
{
    if(cc == NULL || identifier == NULL)
        return NULL;

    return vsc_hashmap_find(&cc->mat_unstyled, identifier);
}

br_material *crocconvert_material_find(CrocConvert *cc, const char *identifier)
{
    br_material *mat;

    if((mat = crocconvert_material_find_styled(cc, cc->current_style, identifier)) != NULL)
        return mat;

    if((mat = crocconvert_material_find_unstyled(cc, identifier)) != NULL)
        return mat;

    return NULL;
}

static int write_materials_mtl(const char *outpath, VscHashMap *hm)
{
    int   r;
    FILE *fp;

    if((r = vsc_fopen(outpath, "wb", &fp)) < 0)
        return r;

    vsc_hashmap_enumerate(hm, mtlwriter, fp);

    (void)fclose(fp);
    return 0;
}

static int write_materials_json(const char *outpath, VscHashMap *hm)
{
    char  *s;
    cJSON *j;
    int    r;
    FILE  *fp;

    if((j = cJSON_CreateObject()) == NULL)
        return VSC_ERROR(ENOMEM);

    if((r = vsc_hashmap_enumerate(hm, mtlwriter_json, j)) < 0)
        return r;

    if((s = cJSON_Print(j)) == NULL)
        return VSC_ERROR(ENOMEM);

    if((r = vsc_fopen(outpath, "wb", &fp)) < 0) {
        cJSON_free(s);
        return r;
    }

    if(fwrite(s, strlen(s), 1, fp) != 1) {
        cJSON_free(s);
        fclose(fp);
    }

    cJSON_free(s);
    (void)fclose(fp);
    return 0;
}

int crocconvert_write_all_materials(CrocConvert *cc)
{
    FILE *fp;
    int   r;

    if((r = vsc_fopen(cc->paths.materials, "wb", &fp)) < 0)
        return r;

    vsc_hashmap_enumerate(&cc->mat_unstyled, mtlwriter, fp);

    (void)fclose(fp);

    if((r = write_materials_json(cc->paths.materials_json, &cc->mat_unstyled)) < 0)
        return r;

    for(size_t i = 0; i < CROC_MAP_STYLE_MAX; ++i) {

        if((r = write_materials_mtl(cc->paths.materials_styled[i], cc->mat_styled + i)) < 0)
            return r;

        if((r = write_materials_json(cc->paths.materials_styled_json[i], cc->mat_styled + i)) < 0)
            return r;
    }

    return 0;
}
