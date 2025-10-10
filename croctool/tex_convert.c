/*
 * CrocUtils - Copyright (C) 2021-2025 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, and only
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <vsclib.h>

#include <brender.h>

#include <libcroc/util.h>

typedef struct Args {
    const char *filename;
    const char *basename;
} Args;

static int parse_args(int argc, char **argv, Args *args)
{
    if(argc == 3) {
        args->filename = argv[1];
        args->basename = argv[2];
        return 0;
    }

    if(argc == 2) {
        args->filename = argv[1];
        args->basename = NULL;
        return 0;
    }

    return -1;
}

int tex_convert(int argc, char **argv)
{
    br_uint_32   num     = 0;
    int          ret     = -1;
    char        *namebuf = NULL;
    const char  *bn_start, *bn_end;
    Args         args;
    br_pixelmap *pixelmaps[128];

    if(parse_args(argc, argv, &args) < 0)
        return 2;

    if(args.basename != NULL) {
        bn_start = args.basename;
        bn_end   = bn_start + strlen(bn_start);
        namebuf  = vsc_asprintf("%s_999.png", bn_start);
    } else {
        const char *filename = croc_util_get_filename(args.filename);
        const char *end      = strrchr(filename, '.');
        if(end == NULL)
            end = filename + strlen(filename);

        bn_start = filename;
        bn_end   = end;
        namebuf  = vsc_asprintf("%.*s_999.png", (int)(bn_end - bn_start), filename);
    }

    if(namebuf == NULL) {
        BrLogError("APP", "Failed to allocate memory: %s", strerror(errno));
        goto done;
    }

    BrBegin();

    num = BrPixelmapLoadMany(args.filename, pixelmaps, BR_ASIZE(pixelmaps));
    BrLogInfo("APP", "loaded %u pixelmaps from %s", num, args.filename);

    if(num == 1) {
        sprintf(namebuf, "%.*s.png", (int)(bn_end - bn_start), bn_start);
        if(!BrFmtImageSave(namebuf, pixelmaps[0], BR_FMT_IMAGE_PNG)) {
            BrLogError("APP", "Error writing subtexture %s: %s", namebuf, strerror(errno));
        }
    } else {
        for(br_uint_32 i = 0; i < num; ++i) {
            sprintf(namebuf, "%.*s_%03u.png", (int)(bn_end - bn_start), bn_start, i);
            if(!BrFmtImageSave(namebuf, pixelmaps[i], BR_FMT_IMAGE_PNG)) {
                BrLogError("APP", "Error writing subtexture %s: %s", namebuf, strerror(errno));
            }
        }
    }

    BrEnd();

    ret = 0;

done:

    if(namebuf != NULL)
        vsc_free(namebuf);

    return ret;
}
