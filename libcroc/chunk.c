/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <errno.h>
#include <vsclib.h>
#include <libcroc/chunk.h>

int croc_chunk_enumerate(FILE *f, CrocChunkEnumerator proc, void *user)
{
    void *buf = NULL;
    int ret = -1, errno_;

    for(;;) {
        CrocChunkType type;
        uint32_t length;
        void *_buf;

        type = vsc_fread_beu32(f);
        length = vsc_fread_beu32(f);

        if(ferror(f) || feof(f)) {
            errno = EIO;
            break;
        }

        //fprintf(stderr, "type = %u, size = %u\n", type, length);
        if(type == 0 && length == 0) {
            ret = 0;
            break;
        }

        if(length > 0) {
            if((_buf = vsc_realloc(buf, length)) == NULL) {
                errno = ENOMEM;
                break;
            }

            buf = _buf;

            if(fread(buf, length, 1, f) != 1) {
                errno = EIO;
                break;
            }
        }

        if(proc(type, buf, length, user) < 0) {
            errno = ECANCELED;
            break;
        }
    }

    errno_ = errno;

    if(buf)
        vsc_free(buf);

    errno = errno_;
    return ret;
}
