#include "defs.h"

int crocconvert_track(CrocConvert *cc, CrocMapStyle style)
{
    CrocModel *models  = NULL;
    size_t     nmodels = 0;
    FILE      *fp;
    int        r;
    char      *path;

    if(style < 0 || style >= CROC_MAP_STYLE_MAX)
        return VSC_ERROR(EINVAL);

    if((path = vsc_asprintf("%s/track/tk%02d_trk.mod", cc->paths.gdata_dir, style)) == NULL)
        return VSC_ERROR(ENOMEM);

    if((r = vsc_fopen(path, "rb", &fp)) < 0)
        goto done;

    if(croc_mod_read_many(fp, &models, &nmodels, CROC_MODEL_TYPE_NORMAL) < 0) {
        r = VSC_ERROR(errno);
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    vsc_free(path);
    path = NULL;

    cc->current_style = style;

    /* Pass 1 - load the materials and textures. */
    for(size_t i = 0; i < nmodels; ++i) {
        CrocModel *mdl = models + i;

        for(size_t f = 0; f < mdl->num_faces; ++f) {
            const CrocModelFace *face = mdl->faces + f;

            /* Trigger BRender to load materials and textures. */
            (void)BrMaterialFind(face->material);
        }
    }

    /* Pass 2 - sort the faces. */
    for(size_t i = 0; i < nmodels; ++i) {
        CrocModel *mdl = models + i;
        qsort(mdl->faces, mdl->num_faces, sizeof(CrocModelFace), crocconvert_face_sort_proc);
    }

    /* Pass 3 - dump them. */
    for(size_t i = 0; i < nmodels; ++i) {
        const CrocModel *mdl = models + i;

        if((path = vsc_asprintf("%s/tk%02d_trk_%03zu.obj", cc->paths.out_track_dir, style, i)) == NULL) {
            r = VSC_ERROR(ENOMEM);
            goto done;
        }

        if((r = crocconvert_write_model(path, mdl, i, nmodels)) < 0) {
            goto done;
        }

        vsc_free(path);
        path = NULL;
    }

    fp = NULL;
    r  = 0;
done:

    if(path != NULL)
        vsc_free(path);

    if(fp != NULL)
        fclose(fp);

    if(models != NULL)
        croc_mod_free_many(models, nmodels);
    return r;
}
