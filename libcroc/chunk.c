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
#include <errno.h>
#include <vsclib.h>
#include <libcroc/chunk.h>

int croc_chunk_enumerate(FILE *f, CrocChunkEnumerator proc, void *user)
{
    void *buf = NULL;
    int ret = -1, /*have_header = 0, */errno_;


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

        /* The file's "magic number" is actually a CROC_CHUNK_HEADER chunk. */
//        if(!have_header) {
//            if(type != CROC_CHUNK_TYPE_HEADER || length != 8) {
//                errno = EINVAL;
//                break;
//            }
//            have_header = 1;
//        }

        if(type == 0 && length == 0) {
            ret = 0;
            break;
        }

        if((_buf = realloc(buf, length)) == NULL) {
            errno = ENOMEM;
            break;
        }

        buf = _buf;

        if(fread(buf, length, 1, f) != 1) {
            errno = EIO;
            break;
        }

        fprintf(stderr, "type = %u, size = %u\n", type, length);
        if(proc(type, buf, length, user) < 0) {
            errno = ECANCELED;
            break;
        }
    }

    errno_ = errno;

    if(buf)
        free(buf);

    errno = errno_;
    return ret;
}