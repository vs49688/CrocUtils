/*
 * CrocUtils - Copyright (C) 2022 Zane van Iperen.
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
#ifndef _LIBCROC_STRUNDEF_H
#define _LIBCROC_STRUNDEF_H

#include <stdint.h>

#define CROC_STRUN_ENTRY_SIZE       36
#define CROC_STRUN_NAME_SIZE        24
#define CROC_STRUN_FILENAME_SIZE     8

/*
 * An entry of "STRUN.BIN".
 * Used to store information about the strategies.
 */
typedef struct CrocStRunEntry {
    char     name[CROC_STRUN_NAME_SIZE + 1];
    char     filename[CROC_STRUN_FILENAME_SIZE + 1];
    uint16_t program_counter;
    uint16_t unknown;
} CrocStRunEntry;

#endif /* _LIBCROC_STRUNDEF_H */