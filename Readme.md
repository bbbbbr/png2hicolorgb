# Note about GBDK-2020
This utility is now part of [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020) (version 4.2.0+), so you may already have a copy if you'r using that dev kit.

# png2hicolorgb
An updated version of Glen Cook's Windows GUI "hicolour.exe" 1.2 conversion tool for the Game Boy Color. The starting code base was the 1.2 release.

# HiColor 
"Hi Color" on the Game Boy Color is a technique for displaying backgrounds with thousands of colors instead being limited to 32 colors for the entire screen background. It achieves this by changing ~16 colors of the background palette per scanline. The main tradeoffs are that it uses much of the Game Boy's available cpu processing per frame and requires more ROM space. The tile patterns, map, attributes and per-scanline palettes are pre-calculated using the PC based conversion tool.

![Hi Color example image on a Game Boy Color](/info/gbc_hicolor_test_pattern.jpg)
![Hi Color test pattern on a Game Boy Color](/info/gbc_hicolor_example_image.jpg)

# GBDK Example Palette ISR
The new palette update ISR in the GBDK example is contributed by [Toxa](https://github.com/untoxa)

# RGBDS Example
RGBDS example modernization contributed by [ISSOtm](https://github.com/ISSOtm)

# Example image
Example image Pixel art originally by RodrixAP under Creative Commons Attribution 2.0 Generic (CC BY 2.0)
https://www.flickr.com/photos/rodrixap/10591266994/in/album-72157637154901153/


# Changes vs original:
- Changed to be a console utility, meant for integration with build toolchains
- Added support for PNG image files
- Added support for multiple OS platforms: Linux, Windows, MacOS
- Added support for images of various heights (8-256 pixels) instead of fixed 144 pixels high
- Added selectable map tile index mode
- Added tile deduplication (including v/h flipped, only beneficial for some images)
- Added precompiled palette loading output (by [ISSOtm](https://github.com/ISSOtm))
- Fixed last scanline tile and palette update missing for Left side of screen
- Removed the "original" mode quantizer and conversion type (Jeff Frohwein/etc, method 0) due to unclear source license status. It was faster, but the other methods tend to have better output.

```
png2hicolorgb input_image.png [options]
version 1.4.2: bbbbbr. Based on Glen Cook's Windows GUI "hicolour.exe" 1.2
Convert an image to Game Boy Hi-Color format

Options
-h         : Show this help
-v*        : Set log level: "-v" verbose, "-vQ" quiet, "-vE" only errors, "-vD" debug
-o <file>  : Set base output filename (otherwise from input image)
--csource  : Export C source format with incbins for data files
--bank=N   : Set bank number for C source output where N is decimal bank number 1-511
--type=N   : Set conversion type where N is one of below 
              1: Median Cut - No Dither (*Default*)
              2: Median Cut - With Dither
              3: Wu Quantiser (best quality)
-p         : Show screen attribute pattern options (no processing)
-L=N       : Set Left  side of screen palette arrangement where N is name listed below or decimal entry
-R=N       : Set Right side of screen palette arrangement where N is name listed below or decimal entry
             Named options for N: "adaptive-fast", "adaptive-medium", "adaptive-best" (-p for full options) 
--best     : Use highest quality conversion settings (--type=3 -L=adaptive-best -R=adaptive-best)
--vaddrid  : Map uses vram id (128->255->0->127) instead of (*Default*) sequential tile order (0->255)
--nodedupe : Turn off tile pattern deduplication
--precompiled   : Export Palette data as pre-compiled executable loading code
--palendbit     : Set unused bit .15 = 1 for last u16 entry in palette data indicating end (not in precompiled)
--addendcolor=N : Append 32 x color N (hex BGR555) in pal data to clear BG for shorter images (64 bytes) (not in precompiled)

Example 1: "png2hicolorgb myimage.png"
Example 2: "png2hicolorgb myimage.png --csource -o=my_output_filename"
Example 2: "png2hicolorgb myimage.png --palendbit --addendcolor=0x7FFF -o=my_output_filename"
* Default settings provide good results. Better quality but slower: "--type=3 -L=adaptive-best -R=adaptive-best"

Historical credits and info:
   Original Concept : Icarus Productions
   Original Code : Jeff Frohwein
   Full Screen Modification : Anon
   Adaptive Code : Glen Cook
   Windows Interface : Glen Cook
   Additional Windows Programming : Rob Jones
   Original Quantiser Code : Benny
   Quantiser Conversion : Glen Cook
```

# Implementation Details
Some notes from reading the original code about how the conversion and display rendering are set up.

- 4 full CGB palettes are updated per scanline
  - (4 palettes x 4 colors per palette x 2 bytes per color = 32 bytes)
- Most palette updates for the next scanline are written starting in HBlank(mode 0) and complete toward the end of OAM Scan(mode 2)
- The two initial palette updates (Left & Right) happen during VBlank(mode 1)

- Only 4 palettes are updated per scanline, so it alternates between updates for the Left and Right side for a given scanline
- This means the Left and Right update regions (sides of the screen) are vertically offset from each other by 1 pixel
- For the conversion process
  - The Left side starts at Y = -1 (clamped to 0), Right side starts at Y = 0
  - The Left side ends at Y = 144 (clamped to 143), Right side ends at Y = 143

- Left side of screen (0-79): allocated palettes 0-3
- Right side of screen (80-159): allocated palettes 4-7
- This ~makes 80 pixel wide x 2 pixel tall regions for the palette updates
- The first and last palette regions on the Left side are 80 x 1 instead of 80 x 2 due to the -1 Y offset
- The width (in 8x8 tiles) for how the Left / Right side palettes are allocated is adjustable (fixed setting or optimized during conversion), but they are always in ascending palette number order.
  - So for the Left side it might be 000-11-222-33, or 00000-1-2-333

- For a 160 x 144 image (full-screen for GB)
  - Palette updates & regions required:
    - (The -1 is clamped to 0, but the Left 80x2 region "starts" at -1)
    - Left side:  73 (Y = -1, 1, 3, ... 141, 143)
    - Right side: 72 (Y =  0, 2, 4, ... 140, 142)

