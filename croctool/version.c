/*
 * CrocUtils - Copyright (C) 2023 Zane van Iperen.
 *    Contact: zane@zanevaniperen.com
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include "config.h"

int version(int argc, char **argv)
{
    fputs("CrocUtils " CROCTOOL_VERSION_STRING " - Copyright (C) 2019-2023 Zane van Iperen\n"
          "  Contact: zane@zanevaniperen.com\n"
          "\n"
          "Built from commit: " CROCTOOL_COMMIT_HASH "\n"
          "\n"
          "SPDX-License-Identifier: MIT\n"
          "\n"
          "Special thanks to Martin Korth (http://problemkaputt.de/) for his\n"
          "assistance with reversing the PSX file formats.\n", stdout);
    return 0;
}
