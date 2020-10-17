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
#include <errno.h>
#include <stdlib.h>
#include <vsclib.h>
#include <libcroc/wad.h>
#include <libcroc/map.h>
#include <libcroc/checksum.h>

static int write_file_base(const char *base, size_t i, void *buf, size_t size)
{
	FILE *f;
	char *name;
	int err = 0, ret = -1;

	if((name = vsc_asprintf("%s%02zu.MAP", base, i)) == NULL) {
		errno = ENOMEM;
		return -1;
	}

	if((f = fopen(name, "wb")) == NULL) {
		err = errno;
		goto done;
	}

	if(fwrite(buf, size, 1, f) != 1) {
		err = EIO;
		goto done;
	}

	ret = 0;
done:
	if(f != NULL)
    (void)fclose(f);

	if(name != NULL)
		free(name);

	errno = err;
	return ret;
}

/*
 * Each map WAD is:
 * struct
 * {
 *     uint32 total_size;
 *     struct
 *     {
 *         uint32 size;
 *         uint8  data[size];
 *         uint32 checksum;
 *     } data[];
 *     uint32 checksum;
 * } wad;
 */
static int extract_mapwad(FILE *f, const char *base)
{
	uint32_t total_size, ck, fck;
	uint8_t *p = NULL, *_p;
	int ret = 1, err = 0;

	total_size = vsc_fread_leu32(f);

	if(ferror(f) || feof(f)) {
		errno = EIO;
		return -1;
	}

	for(size_t i = 0, current = 0; current < total_size; ++i)
	{
		uint32_t size;

		size = vsc_fread_leu32(f);
		if(ferror(f) || feof(f)) {
			err = EIO;
			goto done;
		}

		/* size + 8 == size + data + checksum. */
		if((_p = realloc(p, size + 8)) == NULL) {
			err = ENOMEM;
			goto done;
		}

		p = _p;

		/* Read the data + checksum */
		if(fread(p + 4, size + 4, 1, f) != 1) {
			err = EIO;
			goto done;
		}

		vsc_write_leu32(p, size);

		ck = vsc_read_leu32(p + 4 + size);
		fck = croc_checksum(p, size + 4);

		if(ck != fck) {
			fprintf(stderr, "Subfile %02zu: Checksum mismatch (0x%x != 0x%x)\n", i, ck, fck);
		}


		if(write_file_base(base, i, p, size + 8) < 0)
			goto done;

		current += size + 8;
	}

	ret = 0;

done:

	if(p != NULL)
		free(p);

	errno = err;
	return ret;
}

int map_unwad(int argc, char **argv)
{
	FILE *f;
	long skip;

	if(argc != 3)
		return 2;

	/* Saturn MPLOAD%02u.WADs have an extra size field at the start. */
	if(croc_wad_is_mpload(argv[1], NULL)) {
		skip = 4;
	} else if(croc_wad_is_mapxx(argv[1], NULL)) {
		skip = 0;
	} else {
		fputs("Unable to detect WAD type.\n", stderr);
		return 1;
	}

	if((f = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "Unable to open input file '%s': %s\n", argv[1], strerror(errno));
		return 1;
	}

	if(fseek(f, skip, SEEK_CUR) < 0) {
		fprintf(stderr, "Unable to seek: %s\n", strerror(errno));
		return 1;
	}

	if(extract_mapwad(f, argv[2]) < 0) {
		fprintf(stderr, "Error writing output files: %s\n", strerror(errno));
		fclose(f);
		return 1;
	}

	fclose(f);
	return 0;
}
