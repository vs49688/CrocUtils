#include "defs.h"

static void write_mtl(FILE *fp, br_material *mat)
{
    MaterialUser *user = mat->user;
    float         ka, kd, ks, power;

    ka    = BrScalarToFloat(BrUFractionToScalar(mat->ka));
    kd    = BrScalarToFloat(BrUFractionToScalar(mat->kd));
    ks    = BrScalarToFloat(BrUFractionToScalar(mat->ks));
    power = BrScalarToFloat(mat->power);

    if(power == 1710618.0f) {
        power = 20.0f; /* log2(1710618), also the default power value. */
    }

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

static int mtlwriter(const void *key, void *value, vsc_hash_t hash, void *user)
{
    br_material *mat = value;
    FILE        *fp  = user;

    write_mtl(fp, mat);
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

int crocconvert_write_all_materials(CrocConvert *cc)
{
    FILE *fp;
    int   r;

    if((r = vsc_fopen(cc->paths.materials, "wb", &fp)) < 0)
        return r;

    vsc_hashmap_enumerate(&cc->mat_unstyled, mtlwriter, fp);

    (void)fclose(fp);

    for(size_t i = 0; i < CROC_MAP_STYLE_MAX; ++i) {

        if((r = vsc_fopen(cc->paths.materials_styled[i], "wb", &fp)) < 0)
            return r;

        vsc_hashmap_enumerate(cc->mat_styled + i, mtlwriter, fp);

        (void)fclose(fp);
    }

    return 0;
}
