#ifndef _CROCCONVERT_DEFS_H
#define _CROCCONVERT_DEFS_H

#include <vsclib.h>
#include <brender.h>
#include <libcroc.h>

typedef struct MaterialUser {
    int          is_styled;
    CrocMapStyle style;
    char        *filename;
} MaterialUser;

typedef struct CrocConvert {
    struct {
        char *croc_dir;                                  /* $croc_dir */
        char *gdata_dir;                                 /* $croc_dir/gdata */
        char *wad_bases[CROC_WAD_MAX + 1];               /* $croc_dir/gdata/wadbase */
        char *track_paths[CROC_MAP_STYLE_MAX];           /* $croc_dir/gdata/track/tkXX_trk.mod */
        char *frontend_dir;                              /* $croc_dir/gdata/frontend */
        char *out_dir;                                   /* $out_dir */
        char *materials;                                 /* $out_dir/materials.mtl */
        char *materials_json;                            /* $out_dir/materials.json */
        char *materials_styled[CROC_MAP_STYLE_MAX];      /* $out_dir/materials_$style.mtl */
        char *materials_styled_json[CROC_MAP_STYLE_MAX]; /* $out_dir/materials_$style.json */
        char *materials_sub1;                            /* ../materials.mtl */
        char *materials_styled_sub1[CROC_MAP_STYLE_MAX]; /* ../materials_$style.mtl */
        char *out_track_dir;                             /* $out_dir/track */
        char *out_textures_dir;                          /* $out_dir/textures */
        char *out_frontend_dir;                          /* $out_dir/frontend */
    } paths;

    VscHashMap mat_unstyled;
    VscHashMap mat_styled[CROC_MAP_STYLE_MAX];
    VscHashMap textures;

    CrocWadFs   *wads[CROC_WAD_MAX + 1];
    CrocMapStyle current_style;

} CrocConvert;

#ifdef __cplusplus
extern "C" {
#endif

extern br_filesystem br_wadfs;
extern CrocWadFs    *br_wadfs_set_current(CrocWadFs *w);

extern CrocConvert *current_croc;

/* crocconvert.c */
int crocconvert_init(CrocConvert **cc, const char *croc_dir, const char *out_dir);

/* brender.c */
void crocconvert_configure_brender(CrocConvert *cc);

/* wad.c */
CrocWadFs *crocconvert_get_wadfs_for_current_style(CrocConvert *cc);
CrocWadFs *crocconvert_get_wadfs_for_style(CrocConvert *cc, CrocMapStyle style);
CrocWadFs *crocconvert_get_wadfs(CrocConvert *cc, CrocWad wad);

/* pixelmap.c */
int          crocconvert_pixelmap_register(CrocConvert *cc, const char *identifier, br_pixelmap *pm);
br_pixelmap *crocconvert_pixelmap_find(CrocConvert *cc, const char *identifier);
int          crocconvert_write_all_textures(CrocConvert *cc);

/* texconv.c */
br_pixelmap *crocconvert_to_rgb888(br_pixelmap *pm);

/* material.c */
int crocconvert_material_register_styled(CrocConvert *cc, CrocMapStyle style, const char *identifier, br_material *mat);
int crocconvert_material_register(CrocConvert *cc, const char *identifier, br_material *mat);
br_material *crocconvert_material_find_styled(CrocConvert *cc, CrocMapStyle style, const char *identifier);
br_material *crocconvert_material_find_unstyled(CrocConvert *cc, const char *identifier);
br_material *crocconvert_material_find(CrocConvert *cc, const char *identifier);
int          crocconvert_write_all_materials(CrocConvert *cc);

/* model.c */
int  crocconvert_face_sort_proc(const void *a, const void *b);
int  crocconvert_write_model(const char *pathname, const CrocModel *mdl, size_t idx, size_t count);
void crocconvert_load_model_materials(CrocModel *models, size_t nmodels);
int  crocconvert_load_and_convert_model(CrocConvert *cc, const char *base);

/* track.c */
int crocconvert_track(CrocConvert *cc, CrocMapStyle style);

#ifdef __cplusplus
}
#endif

#endif /* _CROCCONVERT_DEFS_H */