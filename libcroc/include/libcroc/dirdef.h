/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#ifndef _LIBCROC_DIRDEF_H
#define _LIBCROC_DIRDEF_H

#include <stdint.h>

#define CROC_DIRENTRY_SIZE        24
#define CROC_DIRENTRY_NAME_SIZE   12
#define CROC_DIRENTRY_SECTOR_SIZE 2048
#define CROC_DIRENTRY_MAX_FILE_SIZE 16777215

/*
 * A "directory" entry.
 * Used to store CROCFILE.DIR and STRAT.DIR entries.
 *
 * CROCFILE.DIR contains a list of the game files. This seems
 * to be ignored on the PC version. It contains .ENM entries suggesting
 * it's only meaningful in the PSX version.
 *
 * STRAT.DIR contains a list of files that can be loaded by strategies.
 * Tested by selectively removing entries and watching the game fail.
 */
typedef struct CrocDirEntry {
    char     name[CROC_DIRENTRY_NAME_SIZE + 1];
    uint32_t size;
    /*
     * Not sure what this is, always seems to be 0.
     * The DE database field is called "restart", so that's
     * what this is.
     */
    uint8_t  restart;
    /* The offset in the data file. */
    uint32_t offset;
    /*
     * Also not sure what this is. Is only set in old-style
     * directories. Named after DE database field.
     */
    uint32_t usage;
} CrocDirEntry;

#endif /* _LIBCROC_DIRDEF_H */
