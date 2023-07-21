

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

// #define u8  unsigned char
// #define u16 unsigned short
// #define u32 unsigned int
// #define s8  signed char
// #define s16 signed short
// #define s32 signed int

#endif
