
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
// #include "resource.h"
#include "hicolour.h"
#include "jfrohwein.h"
#include "median.h"
#include "wu.h"


#include <common.h>
#include <files.h>
#include <image_info.h>
#include <logging.h>

/* Gameboy Hi-Colour Convertor */
/* Glen Cook */
/* Jeff Frohwein */
/* Rob Jones */


/*

This code is based on the code written by Jeff Frohwein. Jeff originally wrote a 128x128
Gameboy HiColour convertor and made the source publically available. The problem with
the original work, is that the output from the original code had a large white border
making the picture look framed.

The original code was then modified by another party to produce a full screen image, using
a fixed attribute block size of 3-2-3-2-3-2-3-2. The output from this modified code looked
great, but some pictures had artifacts, due to the fixed attribute size.

I then decided to modify the full screen code, to produce pictures with less artifacts, the
attribute blocks are not fixed, they can adapt their size based on the type of picture that
is being converted.

This program will step through every possible combination of attributes to find the best
possible solution.

The program gives the user the option of using fixed or adaptive attribute blocks, fixed
attribute blocks are much quicker to calculate, but the picture quality may not be perfect.

After creating a DOS version of this program, I then went ahead and wrote a windows interface
for it, to tidy it up, and also give me the chance to learn some windows programming. This is
my first windows program, so please be kind.

The best method for converting the pictures, is to use Adaptive method 3, although this can
take quite a bit longer to calculate than the fixed size calculations.

I believe that the new median cut method with dither produces the best results in general,
but the other quantisers can produce better results for other picture types.

I am releasing this program into the public domain, feel free to adapt it in anyway that you
deem fit. I you feel you have improved this program in anyway, drop me a line, and I will
incorperate the changes into newer versions. (GlenCook@hotmail.com)

*/


/* HISTORY */


// V1.0 - 27th March 2000 - First public release
// V1.1 - 30th March 2000 - Rob Jones added seperate thread for conversion process
// V1.2 - 8th  April 2000 - Added other quantisation methods
// V1.4 -            2023 - Converted to cross platform console utility with PNG support (bbbbbr)


// Function prototypes

int    br,bg,bb;

typedef struct
{
    u8        p1;
    u8        p2;
    u8        FileType;
    u8        p3[9];
    u16       XSize;
    u16       YSize;
    u8        BitDepth;
    u8        c1;
    u8        data[160*144][3];
} IMG_TYPE;


// u8            QR[144][160][3];
u8            TileOffset[4];                 // Offset into screen for attribute start
u8            TileWidth[4];                  // No of character attributes width
u8            Pal[8][Y_REGION_COUNT_LR_RNDUP][28][3];             // Palettes for every other line
u8            IdealPal[8][Y_REGION_COUNT_LR_RNDUP][4][3];         // The best fit palette
u8            pic[160][144][3];              // Original Picture
u8            pic2[160][144][3];             // Output picture
u8            out[160][144];                 // Output data

u8            raw[2][160][144][3];           // Original Picture Raw format.
                                             // sourced from [0] = Normal , [1] = GB Color selected by ViewType

// TODO: delete?
s32           ViewType=0;                    // Type of view to show: 0 = Normal , 1 = GB Color

u8            Best[2][Y_HEIGHT_IN_TILES_LR_RNDUP];                   // Best Attribute type to use
// TODO: delete
// s32           GWeight;                  // Colour weighting Green
// s32           RWeight;                  // Colour weighting Red
// s32           BWeight;                  // Colour weighting Blue
// u8            GotFile=0;                     // Is there a file loaded
u8            LConversion;                   // Conversion type for left hand side of the screen
u8            RConversion;                   // Conversion type for right hand side of the screen
// HWND          Ghdwnd;                          // Global window handle
u8            AttribTable[18][20];           // Attribute table for final render
// u8            OldLConv=0;                    // Conversion type
// u8            OldRConv=0;
uint8_t *     pBuffer;
// u8            Message[2000];
s32           ConvertType; //=2;

u8            Data[160*144*3];  // Gets used for quantizing regions. Maybe other things too?

u32           TempD;
s32           BestLine=0;  // TODO: convert to local var
u32           BestQuantLine;
// RGBQUAD       GBView; // converted to local vars



// Shim buffers for the former windows rendered images that were also used for some calculationss
// bmihsource.biWidth          = 160;
// bmihsource.biHeight         = 144;
// bmihsource.biPlanes         = 1;
// bmihsource.biBitCount       = 24;
static      uint8_t Bitsdest[160 * 144 * 3]; // TODO: RGBA 4 bytes per pixel?
static      uint8_t Bitssource[160 * 144 * 3];
//
static      uint8_t *pBitsdest = Bitsdest;
            uint8_t *pBitssource = Bitssource;


#define MAX_CONVERSION_TYPES    83
#define MAX_QUANTISER_TYPES     4


void hicolor_init(void) {
    // Defaults
    LConversion = 3; // Default Conversion (Fixed 3-2-3-2) Left Screen
    RConversion = 3; // Default Conversion (Fixed 3-2-3-2) Righ Screen
    ConvertType = 2; // Normal default is 2 ("Adaptive 3")

    // TODO: unused, delete
    // GWeight=100;
    // RWeight=100;
    // BWeight=100;
}


void hicolor_set_convert_left_pattern(uint8_t new_value) {
    // IDC_CONVERTLEFT
    LConversion = new_value;
    log_verbose("HiColor: Left pattern set to %d\n", new_value);
}


void hicolor_set_convert_right_pattern(uint8_t new_value) {
    // IDC_CONVERTRIGHT
    RConversion = new_value;
    log_verbose("HiColor: Right pattern set to %d\n", new_value);
}


void hicolor_set_type(uint8_t new_value) {
    // IDC_CONVERTTYPE
    ConvertType = new_value;
    log_verbose("HiColor: Convert type set to %d\n", new_value);
}


///////////////////////////////////

// Equivalent of former file loading
static void hicolor_image_import(image_data * p_loaded_image) {
    log_verbose("hicolor_image_import()\n");

// if(CheckTGA()==0)        // Valid File
// {
    // Equivalent of CheckTGA()

    // TODO: input guarding
    // TODO: deduplicate some of the array copying around
    // for (y=0; y<pTGA->YSize; y++)
    uint8_t * p_input_img = p_loaded_image->p_img_data;

    for (int y=0; y< 144; y++) {
        // for (x=0; x<pTGA->XSize; x++)
        for (int x=0; x< 160; x++) {
            // y1 = pTGA->YSize-1-y;

            // b=pTGA->data[count][0];
            // g=pTGA->data[count][1];
            // r=pTGA->data[count++][2];
            // if (yflip)
            //     y1 = y;

            // pic2[x][y1][0] = (u8)(r & 0xf8);
            // pic2[x][y1][1] = (u8)(g & 0xf8);
            // pic2[x][y1][2] = (u8)(b & 0xf8);
            // Clamp to CGB max R/G/B value in RGB 888 mode (31u << 3)

            // png_image[].rgb -> pic2[].rgb -> pBitssource[].bgr??
            pic2[x][y][0] = (p_input_img[RGB_RED]   & 0xf8u);
            pic2[x][y][1] = (p_input_img[RGB_GREEN] & 0xf8u);
            pic2[x][y][2] = (p_input_img[RGB_BLUE]  & 0xf8u);

            p_input_img += RGB_24SZ; // Move to next pixel of source image
        }
    }

    // TODO: It's convoluted, but pBitssource & pBitsdest are used for:
    // - display as windows DIBs (formerly)
    // - and for some calculations at the end of ConvertMethod1()
    for (int y=0; y<144; y++) {
        for (int x=0; x<160; x++) {
            for (int z=0; z<3; z++) {
                // TODO: (2-z) seems to be swapping RGB for BGR?
                *(pBitssource+(143-y)*3*160+x*3+z) = pic2[x][y][2-z];            // Invert the dib, cos windows likes it like that !!
            }
        }
    }

}


// TODO: delete
// // TODO: Don't really care about this, but may need to set the values
// static void hicolor_RGB_sliders(uint8_t r, uint8_t g, uint8_t b) {

//     // TODO:
//     // SendDlgItemMessage(hdwnd,IDC_SLIDER1,TBM_SETRANGE,TRUE,MAKELONG(0,50));            // Set up sliders for RGB
//     // SendDlgItemMessage(hdwnd,IDC_SLIDER1,TBM_SETPOS,1,25);

//     // SendDlgItemMessage(hdwnd,IDC_SLIDER2,TBM_SETRANGE,TRUE,MAKELONG(0,50));
//     // SendDlgItemMessage(hdwnd,IDC_SLIDER2,TBM_SETPOS,1,25);

//     // SendDlgItemMessage(hdwnd,IDC_SLIDER3,TBM_SETRANGE,TRUE,MAKELONG(0,50));
//     // SendDlgItemMessage(hdwnd,IDC_SLIDER3,TBM_SETPOS,1,25);
// }




// TODO: fix
// TODO: Operates on RGB data in pic[] copied from RGB data in pic2
static void hicolor_convert(void) {
    log_verbose("hicolor_convert()\n");

    for(int x=0; x<160; x++)
    {
        for(int y=0; y<144; y++)
        {
            // TODO: Removed RGB weighting. delete it
            // pic[x][y][0]=(u8)(pic2[x][y][0]*RWeight/100);
            // pic[x][y][1]=(u8)(pic2[x][y][1]*GWeight/100);
            // pic[x][y][2]=(u8)(pic2[x][y][2]*BWeight/100);
            pic[x][y][0] = pic2[x][y][0];
            pic[x][y][1] = pic2[x][y][1];
            pic[x][y][2] = pic2[x][y][2];

            for(int i=0; i<3; i++)
            {
                // TODO: delete, without weighting above values will never be > 255
                // if(pic[x][y][i] > 255)
                //     pic[x][y][i] = 255;

                *(Data + y*160*3+x*3+i) = pic[x][y][i];
            }
        }
    }

    if (ConvertType==0)
    {
        ConvertMethod4();

        for(int y=0; y<144; y++)
        {
            for(int x=0; x<160; x++)
            {
                uint8_t col=Picture256[y*160+x];
                for(int z=0; z<3; z++)
                {
                    // TODO: hardwire to normal view raw[0]... and drop ViewType?
                    *(pBitsdest+(143-y)*3*160+x*3+z)=raw[ViewType][x][y][2-z];
                }
            }
        }
    }
    else
    {
        DoOtherConversion(ConvertType-1);
    }
}


static void hicolor_save(const char * fname_base) {
    log_verbose("hicolor_save()\n");
    ExportTileSet(fname_base);
    ExportPalettes(fname_base);
    ExportAttrMap(fname_base);
}


// Currently expects 160x144 x RGB888
void hicolor_process_image(image_data * p_loaded_image, const char * fname_base) {
    log_verbose("hicolor_process_image(), fname_base: \"%s\"\n", fname_base);

    hicolor_image_import(p_loaded_image);
    hicolor_convert();
    hicolor_save(fname_base);
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// TODO: C output mode as well (or could just be header file for use with C wrappers that incbin)

void ExportTileSet(const char * fname_base)
{
    char filename[MAX_PATH*2];
    uint32_t    byteWritten;
    u32      x, y;
    u8       c1,c2;
    u8       dx,dy;
    u8       c;

    log_verbose("Writing Tile\n");
    strcpy(filename, fname_base);
    strcat(filename, ".til");

    #define OUTBUF_SZ_TILES ((144 / 8) * (160 / 8) * 8 * 2) // TODO: make this controllable
    uint8_t output_buf[OUTBUF_SZ_TILES];
    uint8_t * p_buf = output_buf;

    // Write out tilemap data, Left -> Right, Top -> Bottom, 16 bytes per tile
    for (y=0; y<144; y=y+8)
    {
        for (x=0; x<160; x=x+8)
        {
            for (dy=0; dy<8; dy++)
            {
                c1 = 0;
                c2 = 0;
                for (dx=0; dx<8; dx++)
                {
                    c1 = (u8)(c1 << 1);
                    c2 = (u8)(c2 << 1);
                    c = out[x+dx][y+dy];
                    if (c & 2) c1++;
                    if (c & 1) c2++;
                }

                *p_buf++ = c2;
                *p_buf++ = c1;
            }
        }
    }

    if (!file_write_from_buffer(filename, output_buf, OUTBUF_SZ_TILES))
        set_exit_error();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ExportPalettes(const char * fname_base)
{
    char filename[MAX_PATH * 2];
    uint32_t    byteWritten;
    uint8_t     tmpByte;
    s32      i, j, k;
    s32      r,g,b,v;

    strcpy(filename, fname_base);
    strcat(filename, ".pal");
    log_verbose("Writing Palette to \"%s\" (%s)\n", fname_base, filename);

    #define OUTBUF_SZ_PALS (((Y_REGION_COUNT_BOTH_SIDES) * 4 * 4 * 2) + 1) // TODO: make this controllable
    uint8_t output_buf[OUTBUF_SZ_PALS];
    uint8_t * p_buf = output_buf;


    for (i = 0; i < (Y_REGION_COUNT_BOTH_SIDES); i++) // Number of palette sets (left side updates + right side updates)
    {
        for (j = 0; j < 4; j++) // Each palette in the set
        {
            for(k=0; k<4;k++) // Each color in the palette
            {
                r = IdealPal[(i%2)*4+j][i/2][k][0];
                g = IdealPal[(i%2)*4+j][i/2][k][1];
                b = IdealPal[(i%2)*4+j][i/2][k][2];

                // TODO: Converting to BGR555 probably
                v = ((b/8)*32*32) + ((g/8)*32) + (r/8);

                // 2 bytes per color
                *p_buf++ = (u8)(v & 255);
                *p_buf++ = (u8)(v / 256);
            }
        }
    }

    // TODO: What is this and why? :)
    *p_buf++ = 0x2d;

    log_verbose("Writing Palette to \"%s\" (%s)\n", fname_base, filename);
    if (!file_write_from_buffer(filename, output_buf, OUTBUF_SZ_PALS))
        set_exit_error();

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// TODO: split into two functions so it's easier to read
void ExportAttrMap(const char * fname_base)
{
    char    filename[MAX_PATH*2];
    uint32_t   byteWritten;
    s32     i, x, y;
    uint8_t buf, pal;

    log_verbose("Writing Tile ID Map\n");
    strcpy(filename, fname_base);
    strcat(filename, ".map");

    #define OUTBUF_SZ_MAP (20 * 18) // TODO: make this controllable
    uint8_t output_buf_map[OUTBUF_SZ_MAP];


    uint8_t * p_buf_map = output_buf_map;

    for (i = 0; i < (20 * 18); i++)
        *p_buf_map++ = (u8)(((uint8_t) i < 128) ? ((uint8_t)i) + 128 : ((uint8_t)i) - 128);

    if (!file_write_from_buffer(filename, output_buf_map, OUTBUF_SZ_MAP))
        set_exit_error();


    log_verbose("Writing Attribute Map\n");
    strcpy(filename, fname_base);
    strcat(filename, ".atr");

    // Reset pointer to start of file buffer
    p_buf_map = output_buf_map;
    i = 0;

    for (y = 0; y < 18; y++)
    {
        for (x = 0; x < 20; x++)
        {
            pal = (uint8_t)AttribTable[y][x];
            buf = (u8)((i<256) ?  pal :pal|0x08);
            i++;
            *p_buf_map++ = buf;
        }
    }

    if (!file_write_from_buffer(filename, output_buf_map, OUTBUF_SZ_MAP))
        set_exit_error();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: rename to something that aligns with other convert functions
void ConvertMethod4(void)
{

    log_debug("ConvertMethod4()\n");
    u8                StartSplit=0;
    u8                NumSplit=1;
    u16               Steps=1;
    u8                res;


    switch(LConversion)
    {
        case 0:

            StartSplit=0;
            NumSplit=6;
            Steps=126;
            break;

        case 1:

            StartSplit=0;
            NumSplit=10;
            Steps=198;
            break;

        case 2:

            StartSplit=0;
            NumSplit=80;
            Steps=1458;
            break;

        default:

            StartSplit=LConversion-3;
            NumSplit=1;
            Steps=36;
            break;
    }

    switch(RConversion)
    {
        case 0:

            Steps+=108;
            break;

        case 1:

            Steps+=180;
            break;

        case 2:

            Steps+=1440;
            break;

        default:

            Steps+=18;
            break;
    }

    res=DetermineBestLeft(StartSplit,NumSplit);
    RemapGB(CONV_SIDE_LEFT,res,1);

    switch(RConversion)
    {
        case 0:

            StartSplit=0;
            NumSplit=6;
            break;

        case 1:

            StartSplit=0;
            NumSplit=10;
            break;

        case 2:

            StartSplit=0;
            NumSplit=80;
            break;

        default:

            StartSplit=RConversion-3;
            NumSplit=1;
            break;
    }

    RemapGB(CONV_SIDE_RIGHT,StartSplit,NumSplit);
    RemapPCtoGBC();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// This section of code is used to convert an RGB (pc) triplet into a RGB (gameboy)
// triplet. This section of code was kindly donated by Brett Bibby (GameBrains).

uint8_t intensity[32] =
{
 0x00,0x10,0x20,0x30,0x40,0x50,0x5e,0x6c,0x7a,0x88,0x94,0xa0,0xae,0xb7,0xbf,0xc6,
 0xce,0xd3,0xd9,0xdf,0xe3,0xe7,0xeb,0xef,0xf3,0xf6,0xf9,0xfb,0xfd,0xfe,0xff,0xff
};

unsigned char influence[3][3] =
{
    {16,4,4},
    {8,16,8},
    {0,8,16}
};

RGBQUAD translate(uint8_t rgb[3])
{
    RGBQUAD color;
    uint8_t    tmp[3];
    uint8_t    m[3][3];
    uint8_t    i,j;

    for (i=0;i<3;i++)
        for (j=0;j<3;j++)
            m[i][j] = (intensity[rgb[i]>>3]*influence[i][j]) >> 5;

    for (i=0;i<3;i++)
    {
        if (m[0][i]>m[1][i])
        {
            j=m[0][i];
            m[0][i]=m[1][i];
            m[1][i]=j;
        }

        if (m[1][i]>m[2][i])
        {
            j=m[1][i];
            m[1][i]=m[2][i];
            m[2][i]=j;
        }

        if (m[0][i]>m[1][i])
        {
            j=m[0][i];
            m[0][i]=m[1][i];
            m[1][i]=j;
        }

        tmp[i]=(((m[0][i]+m[1][i]*2+m[2][i]*4)*5) >> 4)+32;
    }

    color.rgbRed    = tmp[0];
    color.rgbGreen    = tmp[1];
    color.rgbBlue    = tmp[2];

    return color;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Data table containing all of the possible combinations of attribute blocks
// for one side of the screen.

// The higher the adaptive level, the more combinations of attributes are tested.

u8    SplitData[80][4]=
{
    {3,2,3,2},{2,3,2,3},{2,2,3,3},{2,3,3,2},{3,2,2,3},{3,3,2,2},{4,2,2,2},{2,2,2,4},{2,2,4,2},{2,4,2,2},{1,1,2,6},
    {1,1,3,5},{1,1,4,4},{1,1,5,3},{1,1,6,2},{1,2,1,6},{1,2,2,5},{1,2,3,4},{1,2,4,3},{1,2,5,2},{1,2,6,1},{1,3,1,5},
    {1,3,2,4},{1,3,3,3},{1,3,4,2},{1,3,5,1},{1,4,1,4},{1,4,2,3},{1,4,3,2},{1,4,4,1},{1,5,1,3},{1,5,2,2},{1,5,3,1},
    {1,6,1,2},{1,6,2,1},{2,1,1,6},{2,1,2,5},{2,1,3,4},{2,1,4,3},{2,1,5,2},{2,1,6,1},{2,2,1,5},{2,2,5,1},{2,3,1,4},
    {2,3,4,1},{2,4,1,3},{2,4,3,1},{2,5,1,2},{2,5,2,1},{2,6,1,1},{3,1,1,5},{3,1,2,4},{3,1,3,3},{3,1,4,2},{3,1,5,1},
    {3,2,1,4},{3,2,4,1},{3,3,1,3},{3,3,3,1},{3,4,1,2},{3,4,2,1},{3,5,1,1},{4,1,1,4},{4,1,2,3},{4,1,3,2},{4,1,4,1},
    {4,2,1,3},{4,2,3,1},{4,3,1,2},{4,3,2,1},{4,4,1,1},{5,1,1,3},{5,1,2,2},{5,1,3,1},{5,2,1,2},{5,2,2,1},{5,3,1,1},
    {6,1,1,2},{6,1,2,1},{6,2,1,1}
};



unsigned int ImageRating(u8 *src, u8 *dest, int StartX, int StartY, int Width, int Height)
{
    log_debug("ImageRating()\n");
    unsigned int    tot;
    int                x,y;
    unsigned int    accum=0;
    int                scradd;

    for(y=StartY;y<(StartY+Height);y++)
    {
        for(x=StartX;x<(StartX+Width);x++)
        {
            scradd=(143-y)*(160*3)+x*3;
            tot=(*(src+scradd)-*(dest+scradd)) * (*(src+scradd)-*(dest+scradd));
            tot+=(*(src+scradd+1)-*(dest+scradd+1)) * (*(src+scradd+1)-*(dest+scradd+1));
            tot+=(*(src+scradd+2)-*(dest+scradd+2)) * (*(src+scradd+2)-*(dest+scradd+2));
            accum+=tot;
        }
    }

    return accum;
}


// TODO: rename to something that aligns with other convert functions
void DoOtherConversion(int ConvertType)
{
    log_verbose("DoOtherConversion()\n");
    int        res;
    int        x,y,z,i;
    int        StartSplit=0;
    int        NumSplit=1;
    int        Steps;
    int        MastX,MastY;
    int        Line;
    int        width;

    switch(LConversion)
    {
        case 0:

            StartSplit=0;
            NumSplit=6;
            Steps=504;
            break;

        case 1:

            StartSplit=0;
            NumSplit=10;
            Steps=792;
            break;

        case 2:

            StartSplit=0;
            NumSplit=80;
            Steps=5832;
            break;

        default:

            StartSplit=LConversion-3;
            NumSplit=1;
            Steps=144;
            break;
    }

    switch(RConversion)
    {
        case 0:

            Steps+=504;
            break;

        case 1:

            Steps+=792;
            break;

        case 2:

            Steps+=5832;
            break;

        default:

            Steps+=144;
            break;
    }


    // Convert left side with one extra tile of height to fix
    // the glitching where the last scanline on left bottom region
    // lacks tile and palette data
    res=ConvertMethod1(0,1,0,Y_HEIGHT_IN_TILES_LEFT,StartSplit,NumSplit,ConvertType);        // Step through all options
    ConvertMethod1(0,1,0,Y_HEIGHT_IN_TILES_LEFT,res,1,ConvertType);

    for(y=0;y<189;y++)
        Best[0][y]=res;


    switch(RConversion)
    {
        case 0:

            StartSplit=0;
            NumSplit=6;
            break;

        case 1:

            StartSplit=0;
            NumSplit=10;
            break;

        case 2:

            StartSplit=0;
            NumSplit=80;
            break;

        default:

            StartSplit=RConversion-3;
            NumSplit=1;
            break;
    }

    for(y=0;y<18;y++)
    {
        res=ConvertMethod1(1,1,y,1,StartSplit,NumSplit,ConvertType);        // Step through all options
        ConvertMethod1(1,1,y,1,res,1,ConvertType);
        Best[1][y]=res;
    }


    for(MastX=0;MastX<2;MastX++)
    {
        for(MastY=0;MastY<18;MastY++)
        {
            Line=Best[MastX][MastY];
            width=0;
            for(i=0;i<4;i++)
            {
                TileOffset[i]=width;
                TileWidth[i]=SplitData[Line][i];
                width+=TileWidth[i];
            }

            for(x=0;x<4;x++)
                for(z=TileOffset[x];z<(TileOffset[x]+TileWidth[x]);z++)
                    AttribTable[MastY][MastX*10+z]=x+MastX*4;
        }
    }


    // TODO: fix me -> pBitsdest being used in conversion process
    for(y=0;y<144;y++)
    {
        for(x=0;x<160;x++)
        {
            raw[0][x][y][0] = *(pBitsdest+(143-y)*3*160+x*3+2);
            raw[0][x][y][1] = *(pBitsdest+(143-y)*3*160+x*3+1);
            raw[0][x][y][2] = *(pBitsdest+(143-y)*3*160+x*3);

            RGBQUAD GBView=translate(raw[0][x][y]);

            raw[1][x][y][0] = GBView.rgbRed;
            raw[1][x][y][1] = GBView.rgbGreen;
            raw[1][x][y][2] = GBView.rgbBlue;
        }
    }
    log_progress("\n");
}



// Start X = 0 for Left / 1 for Right
// Width = 1 for half screen 2 = full screen
// StartY = 0 - 17 : Starting attribute block
// Height = Number of attribute blocks to check / process

// TODO: rename to something that aligns with other convert functions
int ConvertMethod1(int StartX, int Width, int StartY, int Height, int StartJ, int FinishJ, int ConvertType)
{
    log_debug("ConvertMethod1()\n");
    u32        Accum,width,x1,ts,tw,y2,x2,y_offset;
    s32        x,y;
    s32        i,j;
    u8        col;


    BestQuantLine=0xffffffff;

    for(x=StartX;x<(StartX+Width);x++)
    {
        // Left side of screen is offset by -1 Y
        // (Left side calcs hang off top and bottom of screen
        // due to Left/Right palette update interleaving)
        if (x == CONV_SIDE_LEFT)
            y_offset = CONV_Y_SHIFT_UP_1;
        else
            y_offset = CONV_Y_SHIFT_NO;

        for(j=StartJ;j<(StartJ+FinishJ);j++)
        {
            Accum=0;
            width=0;
            for(i=0;i<4;i++)
            {
                TileOffset[i]=width;
                TileWidth[i]=SplitData[j][i]<<3;
                width+=TileWidth[i];
            }

            for(y=StartY*4;y<(StartY+Height)*4;y++)
            {
                log_progress(".");

                for(x1=0;x1<4;x1++)
                {
                    ts=TileOffset[x1];
                    tw=TileWidth[x1];

                    for(y2=0;y2<2;y2++)
                    {
                        // Skip if Y line is outside image borders (prevents buffer overflow)
                        // (Left side calcs hang off top and bottom of screen
                        // due to Left/Right palette update interleaving)
                        s32 y_line = (y*2+y2-y_offset);
                        if ((y_line < IMAGE_Y_MIN) || (y_line > IMAGE_Y_MAX)) continue;

                        for(x2=0;x2<tw;x2++)
                        {
                            // i is iterating over r/g/b slots for the current pixel
                            for(i=0;i<3;i++)
                            {
                                *(Data+(tw*3*y2)+x2*3+i) = pic[x*80+ts+x2][y*2+y2-y_offset][i];
                            }
                        }
                    }

                    switch(ConvertType)
                    {
                        case 0:
                            to_indexed(Data,4,0,TileWidth[x1],2);            // Median Reduction No Dither
                            break;

                        case 1:

                            to_indexed(Data,4,1,TileWidth[x1],2);            // Median Reduction With Dither
                            break;

                        case 2:
                            wuReduce(Data,4,TileWidth[x1]*2);                // Wu Reduction
                            break;
                    }

                    for(y2=0;y2<4;y2++)
                    {
                        // Skip if Y is outside allocated Palette size (prevents buffer overflow)
                        // (Left side calcs hang off top and bottom of screen
                        // due to Left/Right palette update interleaving)
                        if (y >= Y_REGION_COUNT_LR_RNDUP) continue;

                        IdealPal[x*4+x1][y][y2][0]=QuantizedPalette[y2][2];
                        IdealPal[x*4+x1][y][y2][1]=QuantizedPalette[y2][1];
                        IdealPal[x*4+x1][y][y2][2]=QuantizedPalette[y2][0];
                    }

                    for(y2=0;y2<2;y2++)
                    {
                        for(x2=0;x2<tw;x2++)
                        {
                            // Skip if Y line is outside image borders (prevents buffer overflow)
                            // since Left side calcs hang off top and bottom of image/screen
                            s32 y_line = (y*2+y2-y_offset);
                            if ((y_line < IMAGE_Y_MIN) || (y_line > IMAGE_Y_MAX)) continue;

                            col=Picture256[y2*tw+x2];
                            out[x*80+x2+ts][y*2+y2-y_offset]=col;

                            for(i=0;i<3;i++)
                            {
                                *(pBitsdest+(143-(y*2+y2-y_offset))*3*160+(x*80+ts+x2)*3+i)=QuantizedPalette[col][i];
                            }
                        }
                    }
                }
            }

            TempD=ImageRating(pBitssource,pBitsdest,StartX*80,StartY*8,Width*80,Height*8);

            if(TempD<BestQuantLine)
            {
                BestLine=j;
                BestQuantLine=TempD;
            }
        }
    }
    return BestLine;
}




