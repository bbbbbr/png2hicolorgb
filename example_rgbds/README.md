# Hi-Color Demo ROM

To build, execute `./build.sh`.

[RGBDS](https://rgbds.gbdev.io) v0.5.0 or later is required.

This example demonstrates using `--precompiled` output mode. In this mode the palette data is exported as executable code for loading the palette instead of raw data. This allows using immediate load instructions which are faster than the popslide method loads, at the expense of the palette portion of the data being about 2x the size.
