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

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <vsclib.h>
#include <libcroc/wad.h>

/* Because I despise templating things in C. */

template <typename T>
T read_le(const void *p) noexcept;

template<>
int8_t read_le<int8_t>(const void *p) noexcept
{
    return vsc_read_int8(p);
}

template<>
int16_t read_le<int16_t>(const void *p) noexcept
{
    return vsc_read_le16(p);
}

template <typename T, size_t base_len>
static int croc_wad_decompress(T *out, const int8_t *in, size_t compressed_size, size_t uncompressed_size) noexcept
{
    const int8_t *in_end  = in  + compressed_size;
    const T      *out_end = out + (uncompressed_size / sizeof(T));

    if(uncompressed_size % sizeof(T)) {
        errno = EINVAL;
        return -1;
    }

    while(in < in_end) {
        /*
         * Read a control byte.
         * - If c <  0, -c uncompressed T's.
         * - if c >= 0, a run of (c + base_len) T's
         */
        int8_t c = *in++;
        if(c < 0) {
            //fprintf(stderr, "C <  0: run of %d bytes\n", -c);
            if(out + -c > out_end) {
                errno = ERANGE;
                return -1;
            }

            memcpy(out, in, -c * sizeof(T));

            out += -c;
            in  += -c * sizeof(T);
        } else {
            //fprintf(stderr, "C >= 0: %d + %zu of %d\n", c, c + base_len, *in);
            if(out + c + base_len > out_end) {
                errno = ERANGE;
                return -1;
            }

            for(int i = 0; i < c + base_len; ++i) {
                /* NB: endianness is a thing -.= */
                //out[i] = *reinterpret_cast<const T*>(in);
                out[i] = read_le<T>(in);
            }

            in  += sizeof(T);
            out += c + base_len;
        }
    }

    return 0;
}

extern "C" int croc_wad_decompressb(void *outbuf, const void *inbuf, size_t compressed_size, size_t uncompressed_size)
{
    return croc_wad_decompress<int8_t, 3>(
        reinterpret_cast<int8_t*>(outbuf),
        reinterpret_cast<const int8_t*>(inbuf),
        compressed_size,
        uncompressed_size
    );
}

extern "C" int croc_wad_decompressw(void *outbuf, const void *inbuf, size_t compressed_size, size_t uncompressed_size)
{
    return croc_wad_decompress<int16_t, 2>(
        reinterpret_cast<int16_t*>(outbuf),
        reinterpret_cast<const int8_t*>(inbuf),
        compressed_size,
        uncompressed_size
    );
}
