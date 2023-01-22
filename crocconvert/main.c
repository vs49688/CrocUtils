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

    crocconvert_load_and_convert_model(cc, "frontend/crocc");
    crocconvert_load_and_convert_model(cc, "frontend/crocroc");
    crocconvert_load_and_convert_model(cc, "frontend/analog");
    crocconvert_load_and_convert_model(cc, "frontend/cd");
    crocconvert_load_and_convert_model(cc, "frontend/cdvolume");
    crocconvert_load_and_convert_model(cc, "frontend/compute");
    crocconvert_load_and_convert_model(cc, "frontend/controll");
    crocconvert_load_and_convert_model(cc, "frontend/credcroc");
    crocconvert_load_and_convert_model(cc, "frontend/crocc");
    crocconvert_load_and_convert_model(cc, "frontend/crocroc");
    crocconvert_load_and_convert_model(cc, "frontend/dolby");
    crocconvert_load_and_convert_model(cc, "frontend/exit");
    crocconvert_load_and_convert_model(cc, "frontend/fxvolume");
    crocconvert_load_and_convert_model(cc, "frontend/gamepad");
    crocconvert_load_and_convert_model(cc, "frontend/grammy");
    crocconvert_load_and_convert_model(cc, "frontend/horn");
    crocconvert_load_and_convert_model(cc, "frontend/memcard");
    crocconvert_load_and_convert_model(cc, "frontend/monitor");
    crocconvert_load_and_convert_model(cc, "frontend/mono");
    crocconvert_load_and_convert_model(cc, "frontend/mute");
    crocconvert_load_and_convert_model(cc, "frontend/next");
    crocconvert_load_and_convert_model(cc, "frontend/pause");
    crocconvert_load_and_convert_model(cc, "frontend/play");
    crocconvert_load_and_convert_model(cc, "frontend/previous");
    crocconvert_load_and_convert_model(cc, "frontend/repeat");
    crocconvert_load_and_convert_model(cc, "frontend/stereo");
    crocconvert_load_and_convert_model(cc, "frontend/stop");
    crocconvert_load_and_convert_model(cc, "frontend/titlebox");
    crocconvert_load_and_convert_model(cc, "frontend/volume");

    crocconvert_write_all_textures(cc);
    crocconvert_write_all_materials(cc);
    return 0;
}
