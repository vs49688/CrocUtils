#include "defs.h"

CrocWadFs *crocconvert_get_wadfs(CrocConvert *cc, CrocWad wad)
{
    if(cc == NULL)
        return NULL;

    return cc->wads[wad];
}

CrocWadFs *crocconvert_get_wadfs_for_style(CrocConvert *cc, CrocMapStyle style)
{
    return crocconvert_get_wadfs(cc, CrocMapStyleToWad[style]);
};

CrocWadFs *crocconvert_get_wadfs_for_current_style(CrocConvert *cc)
{
    return crocconvert_get_wadfs_for_style(cc, cc->current_style);
}
