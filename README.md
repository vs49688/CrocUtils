# CrocUtils

A small collection of utilities for Croc.

The current list is:

* `maptool` - A tool for manipulating Croc .MAP files
* `cfextract` - A tool for extracting `CROCFILE.???` files from the PSX releases

For encoding/decoding of Argonaut ASF and BRP files, please see [FFmpeg](https://ffmpeg.org/).
* ASF decoding is supported as of d84a30e1238b9feed1c957809108fc5e39d80629.
* ASF encoding is supported as of 6fdf3cc53b5e90524ccfde534ae4a91ee87814aa.
* BRP decoding is supported as of a3a6b56200e2899769175291837a178f20e1e95a.

## maptool
### Usage

```
Usage: maptool decompile <input-file.map> [<output-file.json|->]
       maptool compile <input-file.json|-> <output-file.map>
       maptool convert [--rebase [-]<000-899>] <input-file.map> <output-file.map>
       maptool unwad <input-file.wad> <base-name>
```

#### Decompile
Convert any version of a Croc .MAP file into a JSON file for human editing.

#### Compile
Compile a JSON file into a version 21 Croc .MAP file.

#### Convert
Convert any version of a Croc .MAP file into version 21, readable by Croc DE.

* `--rebase` will offset the door targets by the specified number.

#### Unwad

Split a PSX `MAP%d.wad` file into individual .MAP files.

- Will **NOT** work with the `MP%03d_%02d.WAD` files.

#### Notes

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


## cfextract
### Usage
```
Usage: cfextract dump <CROCFILE.DIR> [<output-file.json|->]
       cfextract extract <CROCFILE.DIR> <CROCFILE.DAT|CROCFILE.1> [<outdir>]
```

#### Dump
Dump a CROCFILE.DIR to JSON for human reading.

#### Extract
Extract a CROCFILE.DIR and CROCFILE.{DAT,1} pair into the given directory.
The directory must exist beforehand.

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
