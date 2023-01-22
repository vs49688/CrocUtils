#include <stdlib.h>
#include <libcroc.h>
#include <brender.h>
#include "defs.h"

int dump_track(CrocConvert *cc, int track_id);

int main(int argc, char **argv)
{
    BrBegin();

    int r;

    CrocConvert *cc;

    crocconvert_init(&cc, "/Users/zane/Games/Croc_dev", ".");
    crocconvert_configure_brender(cc);

    for(int i = 0; i < CROC_MAP_STYLE_MAX; ++i) {
        if((r = crocconvert_track(cc, i)) < 0) {
            int xx = 0;
        }
    }

    crocconvert_write_all_textures(cc);
    crocconvert_write_all_materials(cc);
    return 0;
}
