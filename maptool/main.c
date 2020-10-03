/*
 * CrocUtils - Copyright (C) 2020 Zane van Iperen.
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
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <libcroc.h>

#include "cJSON.h"

static void usage(FILE *f, const char *argv0)
{
    fprintf(f, "Usage: %s decompile <input-file.map> [<output-file.json|->]\n", argv0);
    fprintf(f, "       %s compile <input-file.json|-> <output-file.map>\n", argv0);
    fprintf(f, "       %s convert [--rebase [-]<000-899>] <input-file.map> <output-file.map>\n", argv0);
    fprintf(f, "       %s unwad <input-file.wad> <base-name>\n", argv0);
}

static int decompile(int argc, char **argv)
{
    FILE *fp = NULL;
    cJSON *j = NULL;
    char *s = NULL;
    CrocMap map;
    int ret = 1;

    if(argc != 2 && argc != 3)
        return 2;

    croc_map_init(&map);

    if((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if(croc_map_read(fp, &map) < 0) {
        fprintf(stderr, "Error reading map: %s\n", strerror(errno));
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    (void)croc_extract_level_info(argv[1], &map._level, &map._sublevel);

    if(!(j = croc_map_write_json(&map))) {
        fprintf(stderr, "JSON conversion failed: %s\n", strerror(errno));
        goto done;
    }

    if(!(s = cJSON_Print(j))) {
        fprintf(stderr, "String conversion failed: %s\n", strerror(errno));
        goto done;
    }

    if(argc == 3 && strcmp("-", argv[2]) != 0) {
        fp = fopen(argv[2], "w");
    } else {
        fp = stdout;
    }

    if(fp == NULL) {
        fprintf(stderr, "Unable to open output file '%s': %s\n", argv[2], strerror(errno));
        goto done;
    }

    if(fputs(s, fp) == EOF) {
        fprintf(stderr, "Unable to write output file.\n");
        goto done;
    }

    ret = 0;

done:
    if(s != NULL)
        cJSON_free(s);

    if(j != NULL)
        cJSON_Delete(j);

    croc_map_free(&map);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}

#include <vsclib.h>

static int compile(int argc, char **argv)
{
    FILE *fp = NULL;
    void *ptr = NULL;
    size_t len = 0;
    cJSON *j = NULL;
    int ret = 1;
    CrocMap map;

    if(argc != 3)
        return 2;

    croc_map_init(&map);

    if(strcmp("-", argv[1]) == 0)
        fp = stdin;
    else
        fp = fopen(argv[1], "rb");

    if(fp == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        goto done;
    }

    if(vsc_freadall(&ptr, &len, fp) < 0) {
        fprintf(stderr, "Error reading input file: %s\n", strerror(errno));
        goto done;
    }

    if(fp != stdin)
        (void)fclose(fp);

    fp = NULL;

    if((j = cJSON_ParseWithLength(ptr, len)) == NULL) {
        fputs("JSON Parse Error\n", stderr);
        goto done;
    }

    free(ptr);
    ptr = NULL;

    if(croc_map_read_json(j, &map) == NULL) {
        fputs("Invalid map structure\n", stderr);
        goto done;
    }

    if((fp = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr, "Unable to open output file '%s': %s\n", argv[2], strerror(errno));
        goto done;
    }

    if(croc_map_write(fp, &map) < 0) {
        fprintf(stderr, "Unable to write output file: %s\n", strerror(errno));
        goto done;
    }

    ret = 0;

done:

    croc_map_free(&map);

    if(j != NULL)
        cJSON_Delete(j);

    if(ptr != NULL)
        free(ptr);

    if(fp != NULL && fp != stdin)
        (void)fclose(fp);

    return ret;
}

static int convert(int argc, char **argv)
{
    FILE *fp = NULL;
    CrocMap map;
    int ret = 1;
    long rebase;
    const char *in, *out;

    if(argc == 3) {
        in     = argv[1];
        out    = argv[2];
        rebase = 0;
    } else if(argc == 5) {
        char *e = NULL;

        if(strcmp(argv[1], "--rebase") != 0)
            return 2;

        rebase = strtol(argv[2], &e, 10);

        if(errno != 0 || *e != '\0' || rebase < -899 || rebase > 899)
            return 2;

        in  = argv[3];
        out = argv[4];
    } else {
        return 2;
    }


    croc_map_init(&map);

    if((fp = fopen(in, "rb")) == NULL) {
        fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if(croc_map_read(fp, &map) < 0) {
        fprintf(stderr, "Error reading map: %s\n", strerror(errno));
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    if(croc_map_rebase(&map, (int)rebase) < 0) {
        fprintf(stderr, "Unable to rebase map.\n");
        goto done;
    }

    if((fp = fopen(out, "wb")) == NULL) {
        fprintf(stderr, "Unable to open output file '%s': %s\n", argv[2], strerror(errno));
        goto done;
    }

    if(croc_map_write(fp, &map) < 0) {
        fprintf(stderr, "Unable to write output file: %s\n", strerror(errno));
        goto done;
    }

    ret = 0;

done:
    croc_map_free(&map);

    if(fp != NULL)
        (void)fclose(fp);

    return ret;
}

int unwad(int argc, char **argv);

int main(int argc, char **argv)
{
    int ret = 0;

    if(argc < 2) {
        usage(stdout, argv[0]);
        return 2;
    }

    if(!strcmp("decompile", argv[1]))
        ret = decompile(argc - 1, argv + 1);
    else if(!strcmp("convert", argv[1]))
        ret = convert(argc - 1, argv + 1);
    else if(!strcmp("compile", argv[1]))
        ret = compile(argc - 1, argv + 1);
    else if(!strcmp("unwad", argv[1]))
        ret = unwad(argc - 1, argv + 1);
    else
        ret = 2;

    if(ret == 2)
        usage(stderr, argv[0]);

    return ret;

}
