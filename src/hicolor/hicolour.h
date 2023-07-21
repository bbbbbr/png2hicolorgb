#ifndef __hicolour_h__
#define __hicolour_h__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <defines.h>

#include <image_info.h>
#include <logging.h>

extern    u8               TileOffset[4];               // Offset into screen for attribute start
extern    u8               TileWidth[4];                // No of character attributes width
extern    u8               SplitData[80][4];
extern    u8               Pal[8][72][28][3];           // Palettes for every other line
extern    u8               pic[160][144][3];            // Original Picture
extern    u8               IdealPal[8][72][4][3];       // The best fit palette
extern    u8               Best[2][18];                 // Best Attribute type to use
extern    u8               AttribTable[18][20];         // Attribute table for final render
extern    u8               out[160][144];               // Output data
extern    u8               raw[2][160][144][3];         // Original Picture Raw format: 2 x ((160x144) x RGB?)
extern    RGBQUAD          GBView;

void hicolor_init(void);

void hicolor_set_convert_left_pattern(uint8_t new_value);
void hicolor_set_convert_right_pattern(uint8_t new_value);
void hicolor_set_type(uint8_t new_value);

void hicolor_process_image(image_data * p_decoded_image, const char * fname);

/////////////////////////////////////////////////////////////////////////////

void ExportTileSet(const char * fname_base);
void ExportPalettes(const char * fname_base);
void ExportAttrMap(const char * fname_base);

void ConvertMethod4(void);
RGBQUAD translate(uint8_t rgb[3]);
unsigned int ImageRating(u8 *src, u8 *dest, int StartX, int StartY, int Width, int Height);
void DoOtherConversion(int ConvertType);
int ConvertMethod1(int StartX, int Width, int StartY, int Height, int StartJ, int FinishJ, int ConvertType);


#endif
