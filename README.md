# CrocUtils

A small collection of utilities for Croc.

For encoding/decoding of Argonaut ASF and BRP files, please see [FFmpeg](https://ffmpeg.org/).
* ASF decoding is supported as of d84a30e1238b9feed1c957809108fc5e39d80629.
* ASF encoding is supported as of 6fdf3cc53b5e90524ccfde534ae4a91ee87814aa.
* BRP decoding is supported as of a3a6b56200e2899769175291837a178f20e1e95a.

## croctool

### Usage
```
Usage: croctool map decompile <input-file.map> [output-file.json|-]
       croctool map compile <input-file.json|-> <output-file.map>
       croctool map convert [--rebase [-]<000-899>] <input-file.map> <output-file.map>
       croctool map unwad <input-file.wad> <base-name>
       croctool crocfile dump <CROCFILE.DIR> [output-file.json|-]
       croctool crocfile extract <CROCFILE.DIR> <CROCFILE.DAT|CROCFILE.1> [output-dir]
       croctool mod burst2obj <input-file.mod> [output-dir]
       croctool wad extract <base-name> <file-name> [output-dir|-]
       croctool wad extract-all <base-name> [output-dir]
       croctool wad dump-index <index-name.idx> [output-file.json]
       croctool tex convert [--key] <input-file.pix> [base-name]
```

#### map decompile

Convert any version of a Croc .MAP file into a JSON file for human editing.

#### map compile
Compile a JSON file into a version 21 Croc .MAP file.

#### map convert
Convert any version of a Croc .MAP file into version 21, readable by Croc DE.

* `--rebase` will offset the door targets by the specified number.

#### map unwad

Split a PSX `MAP%d.wad` file into individual .MAP files.

- Will **NOT** work with the `MP%03d_%02d.WAD` files.

#### crocfile dump

Dump a CROCFILE.DIR to JSON for human reading.

#### crocfile extract
Extract a CROCFILE.DIR and CROCFILE.{DAT,1} pair into the given directory.
The directory must exist beforehand.

#### mod burst2obj

"Burst" a MOD file into separate Wavefront Alias OBJ meshes.

Material handling isn't supported yet.

#### wad extract

Extract a named file from a given WAD/IDX pair.
* `base-name` should be given as the WAD/IDX path WITHOUT the extension.
* If `output-dir` isn't specified, it will write to the current working directory.
* If `output-dir` is `-`, then the file contents will be written to stdout.
* Otherwise, the file will be placed in `output-dir`.

#### wad extract-all

Extract all files from a given WAD/IDX pair.
* `base-name` should be given as the WAD/IDX path WITHOUT the extension.
* If `output-dir` isn't specified, the files will be written to the current working directory.
* Otherwise, the files will be placed in `output-dir`.

#### wad dump-index

Dump an IDX file to JSON.

#### tex convert

Convert a BRender PIX file (texture) to PNG.
If multiple textures are contained within the same file, the three-digit index
is appended to the file name.

If `--key` is specified and the texture is a 16-bit format, all pure-black (0, 0, 0) pixels
will be marked as completely transparent.

## Notes

* Odd padding in PSX maps
  - There was a batch of PSX maps I didn't extract properly.
    Either I `fopen()`'d the file without `b` and Windows
    added padding, or the iso extraction too messed up.
  - There were sets of `0x0D 0x0A` (or CRLF) randomly in there.
* Extract the ISOs with https://github.com/cebix/psximager/
* All 0.12 beta maps convert fine and are either `v12_old` or `v12_old_noambient`.
* The .MAP files SLUS-00593, SLPS-01055, and SLES-00593 are identical. They are either `normal`, or `v12`

* There are differences between the PSX and PC maps. Don't be alarmed if things look slightly different.
    ```
    $ maptool decompile resources/maps/map/slus00530/MP010_03.MAP  | jq .point_lights[0]
    {
      "position": [
        0.1999969482421875,
        -0.28570556640625,
        2.1999969482421875
      ],
      "colour": [
        255,
        3,
        3
      ],
      "fade_from": 1,
      "fade_to": 1.5
    }
    $ maptool decompile resources/maps/map/pc/MP010_03.MAP  | jq .point_lights[0]
    {
      "position": [
        0,
        -0.4571380615234375,
        2
      ],
      "colour": [
        255,
        83,
        3
      ],
      "fade_from": 1,
      "fade_to": 1.5
    }
    ```

## License

Copyright &copy; 2020 [Zane van Iperen](mailto:zane@zanevaniperen.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2, and only
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

### 3rd-party Licenses

| Project | License | License URL |
| ------- | ------- | ----------- |
| [cJSON](https://github.com/DaveGamble/cJSON) | MIT | https://github.com/DaveGamble/cJSON/blob/master/LICENSE |
