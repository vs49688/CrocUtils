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
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <vsclib.h>
#include <libcroc/mod.h>
#include <libcroc/util.h>

typedef struct LogData
{
    const CrocModel *model;
    size_t          index;
} LogData;

static void logproc(void *user, const char *fmt, ...)
{
    va_list ap;
    LogData *ld = user;

    fprintf(stderr, "submodel %zu, ", ld->index);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

int mod_burst2obj(int argc, char **argv)
{
    FILE *fp = NULL;
    int ret = 1;
    size_t num = 0, bad = 0;
    const char *outdir = ".";
    const char *filename, *ext;
    char *outname = NULL;
    CrocModel *models = NULL;

    if(argc != 2 && argc != 3)
        return 2;

    if(argc == 3)
        outdir = argv[2];

    if(!(fp = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        goto done;
    }

    if(croc_mod_read_many(fp, &models, &num, CROC_MODEL_TYPE_AUTODETECT) < 0) {
        fprintf(stderr, "Unable to read model: %s\n", strerror(errno));
        goto done;
    }

    fclose(fp);
    fp = NULL;

    filename = croc_util_get_filename(argv[1]);
    if((ext = strchr(filename, '.')) == NULL)
        ext = filename + strlen(filename);

    for(size_t i = 0; i < num; ++i) {
        LogData ld = {
            .model = models + i,
            .index = i
        };

        if(croc_mod_validate(models + i, logproc, &ld) < 0)
            ++bad;
    }

    if(bad) {
        fprintf(stderr, "%zu model(s) failed to validate, aborting...\n", bad);
        goto done;
    }

    for(size_t i = 0; i < num; ++i) {
        croc_mod_sort_faces(models + i);

        if((outname = vsc_asprintf("%s/%.*s.%02zu.obj", outdir, (int)(ext - filename), filename, i)) == NULL) {
            fprintf(stderr, "Failed to allocate memory: %s\n", strerror(errno));
            goto done;
        }

#if 0
        {
            fprintf(stderr, "# Rx2012 = %f\n", croc_x2012_to_float(models[i].radius));
            /* For easy visualisation in Matlab/Octave */
            fprintf(stderr, "clf\nhold on\n");
            for(size_t b = 0; b < 9; ++b) {
                fprintf(stderr, "scatter3(%f, %f, %f, \"filled\")\n",
                    croc_x0412_to_float(models[i].bounding_box[b].x),
                    croc_x0412_to_float(models[i].bounding_box[b].y),
                    croc_x0412_to_float(models[i].bounding_box[b].z)
                );
            }
        }
#endif

        if((fp = fopen(outname, "wb")) == NULL) {
            fprintf(stderr, "Failed to open output file '%s': %s\n", outname, strerror(errno));
            goto done;
        }

        fprintf(fp,
            "##\n"
            "# Generated by CrocUtils.\n"
            "#\n"
            "# File:     %s\n"
            "# Submesh:  %zu/%zu\n"
            "# Vertices: %u\n"
            "# Faces:    %u\n"
            "##\n",
            filename, i + 1, num,
            models[i].num_vertices,
            models[i].num_faces
        );

        if(croc_mod_write_obj(fp, models + i, 0) < 0) {
            fprintf(stderr, "Failed to write output file '%s': %s\n", outname, strerror(errno));
            goto done;
        }

        (void)fclose(fp);
        vsc_free(outname);

        fp      = NULL;
        outname = NULL;
    }

    ret = 0;
done:

    if(outname != NULL)
        vsc_free(outname);

    croc_mod_free_many(models, num);

    if(fp != NULL)
        (void)fclose(fp);

    return ret;
}
