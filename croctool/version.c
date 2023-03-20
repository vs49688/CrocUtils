#include <stdio.h>
#include "config.h"

int version(int argc, char **argv)
{
    fputs("CrocUtils " CROCTOOL_VERSION_STRING " - Copyright (C) 2019-2023 Zane van Iperen\n"
          "  Contact: zane@zanevaniperen.com\n"
          "\n"
          "Built from commit: " CROCTOOL_COMMIT_HASH "\n"
          "\n"
          "This program is free software; you can redistribute it and/or modify\n"
          "it under the terms of the GNU General Public License version 2, and only\n"
          "version 2 as published by the Free Software Foundation.\n"
          "\n"
          "Special thanks to Martin Korth (http://problemkaputt.de/) for his\n"
          "assistance with reversing the PSX file formats.\n", stdout);
    return 0;
}
