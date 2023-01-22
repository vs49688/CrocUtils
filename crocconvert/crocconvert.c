#include "defs.h"

int crocconvert_init(CrocConvert **_cc, const char *croc_dir, const char *out_dir)
{
    int          r;
    CrocConvert *cc;

    if(_cc == NULL || croc_dir == NULL || out_dir == NULL)
        return VSC_ERROR(EINVAL);

    if((cc = vsc_calloc(1, sizeof(CrocConvert))) == NULL)
        return VSC_ERROR(ENOMEM);

    /* Build all the paths now 'cause they're bloody painful to deal with. */
    /* FIXME: error checks */
    cc->paths.croc_dir         = vsc_strdup(croc_dir);
    cc->paths.gdata_dir        = vsc_strjoin("/", croc_dir, "gdata", NULL);
    cc->paths.frontend_dir     = vsc_strjoin("/", croc_dir, "gdata", "frontend", NULL);
    cc->paths.out_dir          = vsc_strdup(out_dir);
    cc->paths.materials        = vsc_strjoin("/", out_dir, "materials.mtl", NULL);
    cc->paths.materials_sub1   = vsc_strdup("../materials.mtl");
    cc->paths.out_track_dir    = vsc_strjoin("/", out_dir, "track", NULL);
    cc->paths.out_textures_dir = vsc_strjoin("/", out_dir, "textures", NULL);
    cc->paths.out_frontend_dir = vsc_strjoin("/", out_dir, "frontend", NULL);

    for(size_t i = 1; i < CROC_WAD_MAX + 1; ++i) {
        cc->paths.wad_bases[i] = vsc_strjoin("/", cc->paths.gdata_dir, CrocWadPaths[i], NULL);
    }

    for(size_t i = 0; i < CROC_MAP_STYLE_MAX; ++i) {
        cc->paths.track_paths[i]           = vsc_asprintf("%s/track/tk%02zu_trk.mod", cc->paths.gdata_dir, i);
        cc->paths.materials_styled[i]      = vsc_asprintf("%s/materials_%s.mtl", out_dir, CrocMapStyleStrings[i]);
        cc->paths.materials_styled_sub1[i] = vsc_asprintf("../materials_%s.mtl", CrocMapStyleStrings[i]);
    }

    /* Default style is ICE. */
    cc->current_style = CROC_MAP_STYLE_ICE;

    /* Init our custom "registries". */
    vsc_hashmap_init(&cc->mat_unstyled, vsc_hashmap_string_hash, vsc_hashmap_string_compare);
    for(size_t i = 0; i < CROC_MAP_STYLE_MAX; ++i)
        vsc_hashmap_init(cc->mat_styled + i, vsc_hashmap_string_hash, vsc_hashmap_string_compare);

    vsc_hashmap_init(&cc->textures, vsc_hashmap_string_hash, vsc_hashmap_string_compare);

    /* All in-memory stuff is loaded, now do on-disk things. */

    /* Open the WADs */
    for(size_t i = 1; i < CROC_WAD_MAX + 1; ++i) {
        if((r = croc_wadfs_open(&cc->wads[i], cc->paths.wad_bases[i])) < 0) {
            goto fail;
        }
    }

    if(mkdir(cc->paths.out_dir, 0755) < 0 && errno != EEXIST) {
        r = VSC_ERROR(errno);
        vsc_fperror(stderr, r, "Unable to create directory: %s", cc->paths.out_dir);
        goto fail;
    }

    if(mkdir(cc->paths.out_track_dir, 0755) < 0 && errno != EEXIST) {
        r = VSC_ERROR(errno);
        vsc_fperror(stderr, r, "Unable to create directory: %s", cc->paths.out_dir);
        goto fail;
    }

    if(mkdir(cc->paths.out_textures_dir, 0755) < 0 && errno != EEXIST) {
        r = VSC_ERROR(errno);
        vsc_fperror(stderr, r, "Unable to create directory: %s", cc->paths.out_dir);
        goto fail;
    }

    if(mkdir(cc->paths.out_frontend_dir, 0755) < 0 && errno != EEXIST) {
        r = VSC_ERROR(errno);
        vsc_fperror(stderr, r, "Unable to create directory: %s", cc->paths.out_dir);
        goto fail;
    }

    *_cc = cc;
    return 0;

fail:

    return r;
}
