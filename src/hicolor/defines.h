

#ifndef __defines_h__
#define __defines_h__

#include <stdint.h>

#include <common.h>

// Originates on Windows format is BGRX24
// TODO: is this right & ok for what code expects?
typedef struct tagRGBQUAD {
  uint8_t rgbBlue;
  uint8_t rgbGreen;
  uint8_t rgbRed;
  uint8_t rgbReserved;
} RGBQUAD;

#define RGB_SZ   3 // RGB888 size in bytes

#define u8	uint8_t
#define u16	uint16_t
#define u32 uint32_t
#define s8	int8_t
#define s16 int16_t
#define s32 int32_t

enum conversion_types {
    CONV_TYPE_MED_CUT_NO_DITHER = 1,
    CONV_TYPE_MED_CUT_YES_DITHER = 2,
    CONV_TYPE_WU = 3,

    CONV_TYPE_MIN  = CONV_TYPE_MED_CUT_NO_DITHER,
    CONV_TYPE_MAX  = CONV_TYPE_WU
};

#define CONV_SIDE_LEFT  0
#define CONV_SIDE_RIGHT 1

#define CONV_Y_SHIFT_UP_1    1
#define CONV_Y_SHIFT_NO      0


#define PAL_REGION_HEIGHT_PX 2

// #define BUF_WIDTH      160      // Originally 160
// #define BUF_WIDTH_TILE_MAX      (BUF_WIDTH / TILE_WIDTH_PX)
#define BUF_HEIGHT                  256      // Originally 144
#define BUF_HEIGHT_IN_TILES         (BUF_HEIGHT / TILE_HEIGHT_PX)
#define BUF_HEIGHT_IN_TILES_RNDUP   (BUF_HEIGHT_IN_TILES+1)        // Use larger size[side] for rounded up amount
#define BUF_Y_REGION_COUNT_LR_RNDUP (((BUF_HEIGHT / PAL_REGION_HEIGHT_PX) + 1))

#define VALIDATE_WIDTH  160 // (BUF_WIDTH)
#define VALIDATE_HEIGHT (BUF_HEIGHT)

#endif
