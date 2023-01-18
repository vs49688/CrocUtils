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
#include <stdlib.h>
#include <stdio.h>
#include <brender.h>

static void brend(void)
{
    (void)BrEnd();
}

int mat_toscr(int argc, char **argv)
{
    br_material *mat = NULL;
    const char  *infile, *outfile;
    int ret = 1;

    if(argc != 3)
        return 2;

    infile = argv[1];
    outfile = argv[2];

    BrBegin();

    if((mat = BrMaterialLoad(infile)) == NULL) {
        fprintf(stderr, "Error loading material\n");
        goto done;
    }

    if(BrFmtScriptMaterialSave(outfile, mat) != 1) {
        fprintf(stderr, "Error saving material\n");
        goto done;
    }

    ret = 0;
done:
    if(mat != NULL)
        BrMaterialFree(mat);

    BrEnd();
    return ret;
}
