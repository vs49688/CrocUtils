#include "defs.h"

int crocconvert_track(CrocConvert *cc, CrocMapStyle style)
{
    char namebuf[] = "track/tkXX_trk";

    if(style < 0 || style >= CROC_MAP_STYLE_MAX)
        return VSC_ERROR(EINVAL);

    namebuf[8] = (char)((style / 10) + '0');
    namebuf[9] = (char)((style % 10) + '0');

    cc->current_style = style;
    return crocconvert_load_and_convert_model(cc, namebuf);
}
