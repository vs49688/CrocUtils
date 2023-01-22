#include "defs.h"

CrocConvert *current_croc;

static void material_setup(const char *name, const char *filename, br_material *mat, int is_styled)
{
    MaterialUser *user;
    int           r;

    if(mat->identifier != NULL)
        BrResFree(mat->identifier);

    mat->identifier = BrResStrDup(mat, name);

    user  = BrResAllocate(mat, sizeof(MaterialUser), BR_MEMORY_APPLICATION);
    *user = (MaterialUser){
        .is_styled = is_styled,
        .style     = current_croc->current_style,
        .filename  = BrResStrDup(user, filename),
    };
    mat->user = user;

    if(is_styled)
        r = crocconvert_material_register_styled(current_croc, CROC_MAP_STYLE_MAX, mat->identifier, mat);
    else
        r = crocconvert_material_register(current_croc, mat->identifier, mat);

    vsc_assert(r == 0);
}

static br_material *material_find_failed_load(const char *name)
{
    br_material   *mat;
    br_filesystem *oldfs;
    char          *filename;

    /* Step 1 - see if we have a loaded styled material. */
    if((mat = crocconvert_material_find_styled(current_croc, CROC_MAP_STYLE_MAX, name)) != NULL)
        return mat;

    if((filename = vsc_asprintf("%s.mat", name)) == NULL)
        return NULL;

    /* Step 2 - try to load a styled material. */
    br_wadfs_set_current(crocconvert_get_wadfs_for_current_style(current_croc));
    oldfs = BrFilesystemSet(&br_wadfs);

    if((mat = BrMaterialLoad(filename)) != NULL) {
        material_setup(name, filename, mat, 1);
        vsc_free(filename);
        BrFilesystemSet(oldfs);
        return mat;
    }

    /* Step 3 - see if we have a loaded unstyled material. */
    if((mat = crocconvert_material_find_unstyled(current_croc, name)) != NULL) {
        vsc_free(filename);
        BrFilesystemSet(oldfs);
        return mat;
    }

    /* Step 4 - try to load an unstyled material. */
    br_wadfs_set_current(crocconvert_get_wadfs(current_croc, CROC_WAD_MATERIAL));

    if((mat = BrMaterialLoad(filename)) != NULL) {
        material_setup(name, filename, mat, 0);
        vsc_free(filename);
        BrFilesystemSet(oldfs);
        return mat;
    }

    return NULL;
}

static br_pixelmap *pixelmap_find_failed_load(const char *name)
{
    br_pixelmap   *pm, *pmtemp;
    br_filesystem *oldfs;
    char          *filename;

    if((pm = crocconvert_pixelmap_find(current_croc, name)) != NULL)
        return pm;

    if((filename = vsc_asprintf("%s.pix", name)) == NULL)
        return NULL;

    br_wadfs_set_current(crocconvert_get_wadfs(current_croc, CROC_WAD_TEXTURES));

    oldfs = BrFilesystemSet(&br_wadfs);
    pm    = BrPixelmapLoad(filename);
    BrFilesystemSet(oldfs);

    vsc_free(filename);

    if(pm == NULL)
        return NULL;

    /* De-CLUT if possible. */
    if((pmtemp = BrPixelmapDeCLUT(pm)) != NULL) {
        BrPixelmapFree(pm);
        pm = pmtemp;
    }

    /* Convert it to RGBA8888. */
    if((pmtemp = crocconvert_to_rgb888(pm)) != NULL) {
        BrPixelmapFree(pm);
        pm = pmtemp;
    }

    if(pm->identifier != NULL)
        BrResFree(pm->identifier);

    pm->identifier = BrResStrDup(pm, name);

    crocconvert_pixelmap_register(current_croc, pm->identifier, pm);
    return pm;
}

void crocconvert_configure_brender(CrocConvert *cc)
{
    if(cc == NULL) {
        current_croc = NULL;
        BrMapFindHook(BrMapFindFailedLoad);
        BrMaterialFindHook(BrMaterialFindFailedLoad);
    } else {
        current_croc = cc;
        BrMapFindHook(pixelmap_find_failed_load);
        BrMaterialFindHook(material_find_failed_load);
    }
}
