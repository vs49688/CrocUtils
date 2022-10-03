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
#include <vsclib.h>
#include <libcroc/util.h>

/* FIXME: This should be in vsclib */
#if defined(_MSC_VER)
#   define strcasecmp stricmp
#endif

int map_decompile(int argc, char **argv);
int map_compile(int argc, char **argv);
int map_convert(int argc, char **argv);
int map_unwad(int argc, char **argv);
int crocfile_dump(int argc, char **argv);
int crocfile_extract(int argc, char **argv);
int crocfile_build(int argc, char **argv);
int strun_dump(int argc, char **argv);
int mod_burst2obj(int argc, char **argv);
int wad_extract(int argc, char **argv);
int wad_extract_all(int argc, char **argv);
int wad_dump_index(int argc, char **argv);
int tex_convert(int argc, char **argv);
int psxtex_convert(int argc, char **argv);
int version(int argc, char **argv);

static void usage(FILE *f, const char *argv0)
{
    fprintf(f, "Usage: %s version|-v|--version\n", argv0);
    fprintf(f, "       %s map decompile <input-file.map> [output-file.json|-]\n", argv0);
    fprintf(f, "       %s map compile <input-file.json|-> <output-file.map>\n", argv0);
    fprintf(f, "       %s map convert [--rebase [-]<000-899>] <input-file.map> <output-file.map>\n", argv0);
    fprintf(f, "       %s map unwad <input-file.wad> <base-name>\n", argv0);
    fprintf(f, "       %s crocfile dump <CROCFILE.DIR> [output-file.json|-]\n", argv0);
    fprintf(f, "       %s crocfile extract <CROCFILE.DIR> <CROCFILE.DAT|CROCFILE.1> [output-dir]\n", argv0);
    fprintf(f, "       %s crocfile build <CROCFILE.DIR> <CROCFILE.DAT|CROCFILE.1> <input-file1> [input-file2 [...]]\n", argv0);
    fprintf(f, "       %s strun dump <STRUN.BIN> [output-file.json]\n", argv0);
    fprintf(f, "       %s mod burst2obj <input-file.mod> [output-dir]\n", argv0);
    fprintf(f, "       %s wad extract <base-name> <file-name> [output-dir|-]\n", argv0);
    fprintf(f, "       %s wad extract-all <base-name> [output-dir]\n", argv0);
    fprintf(f, "       %s wad dump-index <index-name.idx> [output-file.json]\n", argv0);
    fprintf(f, "       %s tex convert [--key] <input-file.pix> [base-name]\n", argv0);
    fprintf(f, "       %s psxtex convert [--key] <input-file.bin> [base-name]\n", argv0);
}

static void usage_maptool(FILE *f, const char *argv0)
{
    fprintf(f, "Usage: %s decompile <input-file.map> [output-file.json|-]\n", argv0);
    fprintf(f, "       %s compile <input-file.json|-> <output-file.map>\n", argv0);
    fprintf(f, "       %s convert [--rebase [-]<000-899>] <input-file.map> <output-file.map>\n", argv0);
    fprintf(f, "       %s unwad <input-file.wad> <base-name>\n", argv0);
}

static void usage_cfextract(FILE *f, const char *argv0)
{
    fprintf(f, "Usage: %s dump <CROCFILE.DIR> [output-file.json|-]\n", argv0);
    fprintf(f, "       %s extract <CROCFILE.DIR> <CROCFILE.DAT|CROCFILE.1> [outdir]\n", argv0);
}

static void usage_modtool(FILE *f, const char *argv0)
{
    fprintf(f, "Usage: %s burst2obj <input-file.mod> [output-dir]\n", argv0);
}

static int main_maptool(int argc, char **argv)
{
    int ret = 0;

    if(argc < 2) {
        usage_maptool(stdout, argv[0]);
        return 2;
    }

    if(!strcmp("decompile", argv[1]))
        ret = map_decompile(argc - 1, argv + 1);
    else if(!strcmp("convert", argv[1]))
        ret = map_convert(argc - 1, argv + 1);
    else if(!strcmp("compile", argv[1]))
        ret = map_compile(argc - 1, argv + 1);
    else if(!strcmp("unwad", argv[1]))
        ret = map_unwad(argc - 1, argv + 1);
    else
        ret = 2;

    if(ret == 2)
        usage_maptool(stderr, argv[0]);

    return ret;
}

static int main_cfextract(int argc, char **argv)
{
    int ret = 0;

    if(argc < 2) {
        usage_cfextract(stdout, argv[0]);
        return 2;
    }

    if(!strcmp("dump", argv[1]))
        ret = crocfile_dump(argc - 1, argv + 1);
    else if(!strcmp("extract", argv[1]))
        ret = crocfile_extract(argc - 1, argv + 1);
    else
        ret = 2;

    if(ret == 2)
        usage_cfextract(stderr, argv[0]);

    return ret;
}

static int main_modtool(int argc, char **argv)
{
    int ret = 0;

    if(argc < 2) {
        usage_modtool(stdout, argv[0]);
        return 2;
    }

    if(!strcmp("burst2obj", argv[1]))
        ret = mod_burst2obj(argc - 1, argv + 1);
    else
        ret = 2;

    if(ret == 2)
        usage_modtool(stderr, argv[0]);

    return ret;
}

int main(int argc, char **argv)
{
    int ret = 0;
    const char *argv0name;

    /* Allow invoking as maptool/cfextract/modtool */
    argv0name = croc_util_get_filename(argv[0]);
    if(!strcasecmp("maptool", argv0name))
        return main_maptool(argc, argv);
    else if(!strcasecmp("cfextract", argv0name))
        return main_cfextract(argc, argv);
    else if(!strcasecmp("modtool", argv0name))
        return main_modtool(argc, argv);

    if(argc > 1 && (!strcmp("version", argv[1]) || !strcmp("-v", argv[1]) || !strcmp("--version", argv[1]))) {
        return version(argc - 1, argv + 1);
    }

    if(argc < 3) {
        usage(stdout, argv[0]);
        return 3;
    }

    /* Big-ass multi-level switch ftw. */
    if(!strcmp("map", argv[1])) {
        if(!strcmp("decompile", argv[2]))
            ret = map_decompile(argc - 2, argv + 2);
        else if(!strcmp("convert", argv[2]))
            ret = map_convert(argc - 2, argv + 2);
        else if(!strcmp("compile", argv[2]))
            ret = map_compile(argc - 2, argv + 2);
        else if(!strcmp("unwad", argv[2]))
            ret = map_unwad(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("crocfile", argv[1])) {
        if(!strcmp("dump", argv[2]))
            ret = crocfile_dump(argc - 2, argv + 2);
        else if(!strcmp("extract", argv[2]))
            ret = crocfile_extract(argc - 2, argv + 2);
        else if(!strcmp("build", argv[2]))
            ret = crocfile_build(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("strun", argv[1])) {
        if(!strcmp("dump", argv[2]))
            ret = strun_dump(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("mod", argv[1])) {
        if(!strcmp("burst2obj", argv[2]))
            ret = mod_burst2obj(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("wad", argv[1])) {
        if(!strcmp("extract", argv[2]))
            ret = wad_extract(argc - 2, argv + 2);
        else if(!strcmp("extract-all", argv[2]))
            ret = wad_extract_all(argc - 2, argv + 2);
        else if(!strcmp("dump-index", argv[2]))
            ret = wad_dump_index(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("tex", argv[1])) {
        if(!strcmp("convert", argv[2]))
            ret = tex_convert(argc - 2, argv + 2);
        else
            ret = 2;
    } else if(!strcmp("psxtex", argv[1])) {
        if(!strcmp("convert", argv[2]))
            ret = psxtex_convert(argc - 2, argv + 2);
        else
            ret = 2;
    } else {
        ret = 2;
    }

    if(ret == 2)
        usage(stderr, argv[0]);

    return ret;
}
