/*
 * CrocUtils - Copyright (C) 2021 Zane van Iperen.
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

#include <libcroc/tex.h>
#include <libcroc/chunk.h>
#include <libcroc/col.h>
#include <libcroc/util.h>
#include <vsclib.h>

/* Just until there's somewhere else that needs it. */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

typedef struct Args {
    const char *filename;
    const char *basename;
    int key;
} Args;

static int parse_args(int argc, char **argv, Args *args)
{
    if(argc == 4) {
        if(strcmp("--key", argv[1]) != 0)
            return -1;

        args->filename = argv[2];
        args->basename = argv[3];
        args->key      = 1;
        return 0;
    }

    if(argc == 3) {
        if(strcmp("--key", argv[1]) == 0) {
            args->filename = argv[2];
            args->basename = NULL;
            args->key      = 1;
        } else {
            args->filename = argv[1];
            args->basename = argv[2];
            args->key      = 0;
        }
        return 0;
    }

    if(argc == 2) {
        if(strcmp("--key", argv[1]) == 0)
            return -1;

        args->filename = argv[1];
        args->basename = NULL;
        args->key      = 0;
        return 0;
    }

    return -1;
}

int tex_convert(int argc, char **argv)
{
    CrocTexture *textures[CROC_TEXTURE_MAX_COUNT];
    size_t num = 0;
    FILE *fp = NULL;
    int ret = -1;
    char *namebuf = NULL;
    const char *bn_start, *bn_end;
    CrocColour colour_key = { .r = 0, .g = 0, .b = 0, .pad = 0 };
    Args args;

    if(parse_args(argc, argv, &args) < 0)
        return 2;

    if(args.basename != NULL) {
        bn_start = args.basename;
        bn_end   = bn_start + strlen(bn_start);
        namebuf  = vsc_asprintf("%s_999.png", bn_start);
    } else {
        const char *filename = croc_util_get_filename(args.filename);
        const char *end = strrchr(filename, '.');
        if(end == NULL)
            end = filename + strlen(filename);

        bn_start = filename;
        bn_end   = end;
        namebuf  = vsc_asprintf("%.*s_999.png", (int)(bn_end - bn_start), filename);
    }

    if(namebuf == NULL) {
        fprintf(stderr, "Failed to allocate memory: %s\n", strerror(errno));
        goto done;
    }

    if((fp = fopen(args.filename, "rb")) == NULL) {
        fprintf(stderr, "Unable to open PIX file '%s': %s\n", args.filename, strerror(errno));
        goto done;
    }

    if(croc_texture_read_many(fp, textures, &num) < 0) {
        fprintf(stderr, "Error reading textures: %s\n", strerror(errno));
        goto done;
    }

    fclose(fp);
    fp = NULL;

    for(size_t i = 0; i < num; ++i) {
        CrocTexture *tex = textures[i];
        uint32_t *data;

        switch(tex->format) {
            case CROC_TEXFMT_INDEX8:
                if(tex->palette == NULL) {
                    fprintf(stderr, "Indexed texture without palette (index=%zu). This is currently not supported.\n", i);
                    continue;
                }
                if((tex = croc_texture_deindex8(tex, args.key ? &colour_key : NULL)) == NULL) {
                    fprintf(stderr, "Error converting to RGBA8888: %s\n", strerror(errno));
                    goto done;
                }
                croc_texture_free(textures[i]);
                textures[i] = tex;
                goto is_rgba8888;

            case CROC_TEXFMT_RGB888:
                if((tex = croc_texture_rgb888_to_rgba8888(tex, args.key ? &colour_key : NULL)) == NULL) {
                    fprintf(stderr, "Error converting to RGBA8888: %s\n", strerror(errno));
                    goto done;
                }
                croc_texture_free(textures[i]);
                textures[i] = tex;
                goto is_rgba8888;

            case CROC_TEXFMT_XRGB1555:
                croc_texture_xrgb1555_to_rgb565(tex);
                /* FALL THROUGH */

            case CROC_TEXFMT_RGB565:
                /* Convert it to 32-bit RGBA */
                if((tex = croc_texture_rgb565_to_rgba8888(tex, args.key ? &colour_key : NULL)) == NULL) {
                    fprintf(stderr, "Error converting to RGBA8888: %s\n", strerror(errno));
                    goto done;
                }

                croc_texture_free(textures[i]);
                textures[i] = tex;

                /* FALL THROUGH */
is_rgba8888:
            case CROC_TEXFMT_RGBA8888:
                /* Convert it in-place to something stbi can handle. */
                data = tex->data;
                tex->format = CROC_TEXFMT_RGBA8888_ARR;
                for(size_t p = 0; p < tex->width * tex->height; ++p)
                    data[p] = vsc_native_to_beu32(data[p]);

                /* FALL THROUGH */

            case CROC_TEXFMT_RGBA8888_ARR:
                break;

            default:
                /* NB: libcroc will warn about this. */
                continue;
        }

        if(num == 1)
            sprintf(namebuf, "%.*s.png", (int)(bn_end - bn_start), bn_start);
        else
            sprintf(namebuf, "%.*s_%03zu.png", (int)(bn_end - bn_start), bn_start, i);

        if(!stbi_write_png(namebuf, tex->width, tex->height, 4, tex->data, tex->bytes_per_row)) {
            fprintf(stderr, "Error writing subtexture %s: %s\n", namebuf, strerror(errno));
        }
    }

    ret = 0;

done:
    if(fp != NULL)
        fclose(fp);

    if(namebuf != NULL)
        free(namebuf);

    croc_texture_free_many(textures, num);

    return ret;
}
