# png2hicolorgb

An updated version of Glenn Cook's Windows Game Boy Hi Colour conversion app. The starting code base was the 1.2 release.

# Example image
Example image Pixel art originally by RodrixAP under Creative Commons Attribution 2.0 Generic (CC BY 2.0)
https://www.flickr.com/photos/rodrixap/10591266994/in/album-72157637154901153/


# Improvements:
- Support for PNG image files
- Support for multiple OS platforms: Linux, Windows, MacOS
- Console based, meant for integration with build toolchains
- Fixes last scanline tile and palette update missing for Left side of screen
  - (For conversion methods 1-3, a few glitches still remain for method 0)


```
png2hicolorgb input_image.png [options]
version 1.4.0: bbbbbr. Based on Glen Cook Windows hicolor.exe 1.2
Convert an image to Game Boy Hi-Color format

Options
-h   : Show this help
-o=* : Set base output filename (otherwise filename from input image)
-v*  : Set log level: "-v" verbose, "-vQ" quiet, "-vE" only errors, "-vD" debug
-T=N : Set conversion type where N is one of below 
        0: Original (J.Frohwein)
        1: Median Cut - No Dither (*Default*)
        2: Median Cut - With Dither
        3: Wu Quantiser
-L=N : Set Left  screen attribute pattern where N is decimal entry (-p to show patterns)
-R=N : Set Right screen attribute pattern where N is decimal entry (-p to show patterns)
-p   : Show screen conversion attribute pattern list (no processing)

Example 1: "png2hicolorgb myimage.png"
Example 2: "png2hicolorgb myimage.png -M=3 -L=2 -R=2 -o:my_base_output_filename"

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

