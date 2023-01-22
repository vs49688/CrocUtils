#include "defs.h"

///* Just until there's somewhere else that needs it. */
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "../croctool/stb_image_write.h"

static int pixwriter(const void *key, void *value, vsc_hash_t hash, void *user)
{
    br_pixelmap *pm = value;
    CrocConvert *cc = user;
    char        *path;

    vsc_assert(pm->type == BR_PMT_RGB_888);

    path = vsc_asprintf("%s/%s.tga", cc->paths.out_textures_dir, pm->identifier);

    //    char *pathp = vsc_asprintf("%s/%s.pix", cc->paths.out_textures_dir, pm->identifier);

    BrFilesystemSet(NULL);
    //    BrPixelmapSave(pathp, pm);

    // stbi_write_png(path, pm->width, pm->height, 3, pm->pixels, pm->row_bytes);
    char *T_SaveTarga(br_pixelmap * pm, const char *target);
    T_SaveTarga(pm, path);
    return 0;
}

int crocconvert_pixelmap_register(CrocConvert *cc, const char *identifier, br_pixelmap *pm)
{
    if(cc == NULL || identifier == NULL)
        return VSC_ERROR(EINVAL);

    if(pm == NULL)
        return 0;

    return vsc_hashmap_insert(&cc->textures, identifier, pm);
}

br_pixelmap *crocconvert_pixelmap_find(CrocConvert *cc, const char *identifier)
{
    if(cc == NULL || identifier == NULL)
        return NULL;

    return vsc_hashmap_find(&cc->textures, identifier);
}

int crocconvert_write_all_textures(CrocConvert *cc)
{
    return vsc_hashmap_enumerate(&cc->textures, pixwriter, cc);
}
