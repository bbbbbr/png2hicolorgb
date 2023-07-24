# png2hicolorgb

An updated version of Glenn Cook's Windows Game Boy Hi Colour conversion app. The starting code base was the 1.2 release.

# Improvements:
- Support for PNG image files
- Support for multiple OS platforms: Linux, Windows, MacOS
- Console based, meant for integration with build toolchains
- Fixes last scanline tile and palette update missing for Left side of screen
  - (For conversion methods 1-3, a few glitches still remain for method 0)

# Implementation Details
Some notes from reading the original code about how the conversion and display rendering are set up.

- 4 full CGB palettes are updated per scanline
  - (4 palettes x 4 colors per palette x 2 bytes per color = 32 bytes)
- Most palette updates for the next line are written starting in HBlank(mode 0) and complete toward the end of OAM Scan(mode 2)
- The two initial palette updates (Left & Right) happen during VBlank(mode 1)

- Since only 4 palettes are updated per scanline, so it alternates between updates for the Left and Right side for a given scanline
- This means the Left and Right update regions (sides of the screen) are vertically offset from each other by 1 scanline
- For the conversion process
  - The Left side starts at Y = -1, Right side starts at Y = 0
  - The Left side ends at Y = 144, Right side starts at Y = 143

- Left side of screen (0-79): allocated palettes 0-3
- Right side of screen (80-159): allocated palettes 4-7
- This ~makes 80 pixel wide x 2 pixel tall regions for the palette updates
- The first and last palette regions on the Left side are 80 x 1 instead of 80 x 2 due to the -1 Y offset
- The tile width of how the 4 palettes for the Left / Right side are allocated for is adjustable (fixed setting or optimized during conversion) on the GB tile & attribute grid (8x8), but they are always in ascending palette number order.
  - So for the Left side it might be 000-11-222-33, or 00000-1-2-333

- For a 160 x 144 image (full-screen for GB)
  - Palette updates & regions required:
    - Left side: 73
    - Right side: 72

