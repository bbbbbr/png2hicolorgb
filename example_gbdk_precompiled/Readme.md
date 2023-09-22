# GBDK Example -with Precompiled mode
This example Requires GBDK-2020 4.2.0 or higher (due to using `vsync()`)

PNG images placed in the res/hicolor folder will get automatically converted and compiled.

To use them, include the automatically generated header file (based on the png filename) and then use the `hicolor_start()` and `hicolor_stop()` functions.

# Precompiled mode
This example demonstrates using `--precompiled` output mode. In this mode the palette data is exported as executable code for loading the palette instead of raw data. This allows using immediate load instructions which are faster than the popslide method loads, at the expense of the palette portion of the data being about 2x the size.

# Example image
Example image Pixel art originally by RodrixAP under Creative Commons Attribution 2.0 Generic (CC BY 2.0)
https://www.flickr.com/photos/rodrixap/10591266994/in/album-72157637154901153/

