/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
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
