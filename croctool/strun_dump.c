/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <string.h>
#include <vsclib.h>
#include <cJSON.h>
#include <libcroc/strun.h>

int strun_dump(int argc, char **argv)
{
    FILE *fp = NULL;
    cJSON *j = NULL;
    char *s = NULL;
    int ret = 1;
    CrocStRunEntry *entries = NULL;
    size_t dcount = 0;

    if(argc != 2 && argc != 3)
        return 2;

    if((fp = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr, "Unable to open STRUN.BIN '%s': %s\n", argv[1], strerror(errno));
        return 1;
    }

    if((entries = croc_strun_read(fp, &dcount)) == NULL) {
        fprintf(stderr, "Unable to read STRUN.BIN: %s\n", strerror(errno));
        goto done;
    }

    (void)fclose(fp);
    fp = NULL;

    if(!(j = croc_strun_write_json(entries, dcount))) {
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

    if(entries != NULL)
        vsc_free(entries);

    if(fp != NULL && fp != stdout)
        (void)fclose(fp);

    return ret;
}
