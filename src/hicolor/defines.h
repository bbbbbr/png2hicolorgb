

#ifndef __defines_h__
#define __defines_h__

#include <stdint.h>

// Originates on Windows format is BGRX24
// TODO: is this right & ok for what code expects?
typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

#define u8	uint8_t
#define u16	uint16_t
#define u32 uint32_t
#define s8	int8_t
#define s16 int16_t
#define s32 int32_t

enum conversion_types {
    CONV_TYPE_ORIG_JEFF = 0,
    CONV_TYPE_MED_CUT_NO_DITHER = 1,
    CONV_TYPE_MED_CUT_YES_DITHER = 2,
    CONV_TYPE_WU = 3,

    CONV_TYPE_MIN  = CONV_TYPE_ORIG_JEFF,
    CONV_TYPE_MAX  = CONV_TYPE_WU
};

#define CONV_SIDE_LEFT  0
#define CONV_SIDE_RIGHT 1

#define CONV_Y_SHIFT_UP_1    1
#define CONV_Y_SHIFT_NO      0

#define IMAGE_Y_MIN          0
#define IMAGE_Y_MAX          143
#define IMAGE_HEIGHT         ((IMAGE_Y_MAX - IMAGE_Y_MIN) + 1)// 144
#define TILE_HEIGHT_PX       8
#define PAL_REGION_HEIGHT_PX 2

// Screen palette region updates are 80 pixels wide and 2 pixels tall
// since palette 0-3 allocated to left side, 4-7 allocated to right side
// and only 4 palettes are updated per scanline, so Left and Right alternate in gettig udpates
// 73(L) & 72(R) for standard GB screen
#define Y_REGION_COUNT_LEFT         ((IMAGE_HEIGHT / PAL_REGION_HEIGHT_PX) + 1)  // One extra region due to starting at -1 Y offset from screen grid, and so there is a last extra entry that "hangs off" the bottom of the screen
#define Y_REGION_COUNT_RIGHT        (IMAGE_HEIGHT / PAL_REGION_HEIGHT_PX)
#define Y_REGION_COUNT_LR_RNDUP     (Y_REGION_COUNT_LEFT) // Use larger size[side] for rounded up amount
#define Y_REGION_COUNT_BOTH_SIDES   (Y_REGION_COUNT_LEFT + Y_REGION_COUNT_RIGHT)

// 19(L) & 18(R) for standard GB screen
#define Y_HEIGHT_IN_TILES_LEFT          ((IMAGE_HEIGHT / TILE_HEIGHT_PX) + 1)  // One extra region due to starting at -1 Y offset from screen grid, and so there is a last extra entry that "hangs off" the bottom of the screen
#define Y_HEIGHT_IN_TILES_RIGHT         (IMAGE_HEIGHT / TILE_HEIGHT_PX)
#define Y_HEIGHT_IN_TILES               (IMAGE_HEIGHT / TILE_HEIGHT_PX)
#define Y_HEIGHT_IN_TILES_LR_RNDUP      (Y_HEIGHT_IN_TILES_LEFT) // Use larger size[side] for rounded up amount

// #define u8  unsigned char
// #define u16 unsigned short
// #define u32 unsigned int
// #define s8  signed char
// #define s16 signed short
// #define s32 signed int

#endif
