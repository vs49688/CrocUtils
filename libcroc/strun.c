/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <vsclib.h>
#include <cJSON.h>
#include <libcroc/strun.h>

static int croc_strun_read_entry(CrocStRunEntry *e, FILE *f)
{
    uint8_t buf[CROC_STRUN_ENTRY_SIZE];

    if(fread(buf, CROC_STRUN_ENTRY_SIZE, 1, f) != 1) {
        errno = EIO;
        return -1;
    }

    memcpy(e->name, buf, CROC_STRUN_NAME_SIZE);
    e->name[CROC_STRUN_NAME_SIZE] = '\0';

    memcpy(e->filename, buf + CROC_STRUN_NAME_SIZE, CROC_STRUN_FILENAME_SIZE);
    e->filename[CROC_STRUN_FILENAME_SIZE] = '\0';

    e->program_counter = vsc_read_leu16(buf + 32);
    e->unknown         = vsc_read_leu16(buf + 34);

    return 0;
}

CrocStRunEntry *croc_strun_read(FILE *f, size_t *count)
{
    CrocStRunEntry *entries = NULL;
    uint32_t size;
    int err;

    if(f == NULL) {
        errno = EINVAL;
        return NULL;
    }

    size = vsc_fread_leu32(f);

    if(feof(f) || ferror(f)) {
        errno = EIO;
        goto done;
    }

    size /= CROC_STRUN_ENTRY_SIZE;

    if((entries = vsc_calloc(size, sizeof(CrocStRunEntry))) == NULL)
        goto done;

    for(uint32_t i = 0; i < size; ++i) {
        if(croc_strun_read_entry(entries + i, f) < 0)
            goto done;
    }

    *count = size;
    return entries;
done:
    err = errno;

    if(entries != NULL)
        vsc_free(entries);

    errno = err;
    return NULL;
}

cJSON *croc_strun_write_json(const CrocStRunEntry *entries, size_t count)
{
    cJSON *jarr, *jent;

    if((jarr = cJSON_CreateArray()) == NULL)
        return NULL;

    for(size_t i = 0; i < count; ++i) {
        const CrocStRunEntry *e = entries + i;

        if((jent = cJSON_CreateObject()) == NULL)
            goto fail;

        if(!cJSON_AddItemToArray(jarr, jent))
            goto fail;

        if(cJSON_AddStringToObject(jent, "name", e->name) == NULL)
            goto fail;

        if(cJSON_AddStringToObject(jent, "filename", e->filename) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "program_counter", e->program_counter) == NULL)
            goto fail;

        if(cJSON_AddNumberToObject(jent, "unknown", e->unknown) == NULL)
            goto fail;
    }

    return jarr;

fail:
    cJSON_Delete(jarr);
    return NULL;
}
