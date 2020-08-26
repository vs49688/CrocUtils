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
#ifndef _LIBCROC_WAD_H
#define _LIBCROC_WAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Is the file a Saturn MPLOAD%02u.WAD? */
int croc_wad_is_mpload(const char *path, unsigned int *level);

/* Is the file a PSX MAP%02u.WAD? */
int croc_wad_is_mapxx(const char *path, unsigned int *level);

#ifdef __cplusplus
}
#endif

#endif /* _LIBCROC_WAD_H */