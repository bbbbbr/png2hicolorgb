
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "jfrohwein.h"
#include "hicolour.h"



u8                TempPal[80][4][4][4][3];            // Temporary palette
s32                avgr,avgg,avgb;                        // Colour averages


void RemapGB(u8 MastX, u8 StartSplit, u8 NumSplit)
{
    log_debug("RemapGB()\n");
    u32        closest;
    u32        a1,b1,c1,d1,x,y,dx,dx2,dy;
    s32        cx,cy,y2;
    s32        sx,sx2,sx3;

    s32        m1,m2,m3,m4;
    s32        p1=0,p2=0,p3=0,p4=0;
    s32        SmallestError;
    s32        os;
    s32        state;
    s32        MastY;
    s32        sp;
    u8        j;

    s32        ErrorTerm;
    u32        Accum;
    u8        width,i;
    u8        Line=0;
    u32        BestSoFar;


       for(MastY=0;MastY<18;MastY++)
       {
        BestSoFar=0x7fffffff;

        for(j=StartSplit;j<(StartSplit+NumSplit);j++)
        {
            log_progress(".");
            Accum=0;
            width=0;
            for(i=0;i<4;i++)
            {
                TileOffset[i]=width;
                TileWidth[i]=SplitData[j][i]<<3;
                width+=TileWidth[i];
            }

            for (x=0; x<4; x++ )
            {
                for (y=0; y<4; y++)
                {
                    y2 = (MastY*8+y*2)/2;        //setup Y-index into Pal structure.

                    sx = TileOffset[x]+MastX*80;
                    sx2 = sx + TileWidth[x];

                    for (sx3=0;sx<sx2;sx+=2 )
                    {
                        AddPixels(sx,MastY*8+y*2,2,2);    //bilerp 2*2 or 3*2 texel.

                        Pal[x+MastX*4][y2][sx3][0] = (unsigned char)avgr;
                        Pal[x+MastX*4][y2][sx3][1] = (unsigned char)avgg;
                        Pal[x+MastX*4][y2][sx3][2] = (unsigned char)avgb;
                        sx3++;
                    }
                }
            }

            for (x=0; x<4; x++ )
            {
                for (y=0; y<4; y++)
                {
                    SmallestError = 0xffffff;
                    state=0;

                    if(MastX==0)
                        os=1;
                    else
                        os=0;

                    y2 = (MastY*8+y*2)/2;        //setup Y-index into Pal structure.

                    sx = TileWidth[x];
                    sp = TileOffset[x]+MastX*80;

                    if ( CountColorsInCell(sp,MastY*8+y*2,sx,os) > 4 )
                    {
                        sx = sx/2;

                        //try all combinations to find lowest error-rate.

                        for(m4=0; m4<sx; m4++)
                        {
                            for(m3=0; m3<sx; m3++)
                            {
                                for(m2=0; m2<sx; m2++)
                                {
                                    for(m1=0; m1<sx; m1++)
                                    {
                                        if ( (m4 > m3) && (m3 > m2) && (m2 > m1) ) //avoid duplicate checks.
                                        {
                                            ErrorTerm = 0;    //clear total error.

                                            for(dy=0; dy<2; dy++) //scan 2 rows.
                                            {
                                                for(dx=0; dx<TileWidth[x]; dx++)    //scan column of tile.
                                                {
                                                    cx = TileOffset[x]+MastX*80 + dx;   //get X-index into image.

                                                    cy = MastY*8+y*2-os+dy; //get row (Y) index.

                                                    if(cy<0) cy=0;

                                                    a1= (Pal[MastX*4+x][y2][m1][0]-(int)pic[cx][cy][0]) * (Pal[MastX*4+x][y2][m1][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[MastX*4+x][y2][m1][1]-(int)pic[cx][cy][1]) * (Pal[MastX*4+x][y2][m1][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[MastX*4+x][y2][m1][2]-(int)pic[cx][cy][2]) * (Pal[MastX*4+x][y2][m1][2]-(int)pic[cx][cy][2]) ;

                                                    b1= (Pal[MastX*4+x][y2][m2][0]-(int)pic[cx][cy][0]) * (Pal[MastX*4+x][y2][m2][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[MastX*4+x][y2][m2][1]-(int)pic[cx][cy][1]) * (Pal[MastX*4+x][y2][m2][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[MastX*4+x][y2][m2][2]-(int)pic[cx][cy][2]) * (Pal[MastX*4+x][y2][m2][2]-(int)pic[cx][cy][2]) ;

                                                    c1= (Pal[MastX*4+x][y2][m3][0]-(int)pic[cx][cy][0]) * (Pal[MastX*4+x][y2][m3][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[MastX*4+x][y2][m3][1]-(int)pic[cx][cy][1]) * (Pal[MastX*4+x][y2][m3][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[MastX*4+x][y2][m3][2]-(int)pic[cx][cy][2]) * (Pal[MastX*4+x][y2][m3][2]-(int)pic[cx][cy][2]) ;

                                                    d1= (Pal[MastX*4+x][y2][m4][0]-(int)pic[cx][cy][0]) * (Pal[MastX*4+x][y2][m4][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[MastX*4+x][y2][m4][1]-(int)pic[cx][cy][1]) * (Pal[MastX*4+x][y2][m4][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[MastX*4+x][y2][m4][2]-(int)pic[cx][cy][2]) * (Pal[MastX*4+x][y2][m4][2]-(int)pic[cx][cy][2]) ;


                                                    closest = d1;

                                                    if (c1 < closest)
                                                        closest = c1;

                                                    if (b1 < closest)
                                                        closest = b1;

                                                    if (a1 < closest)
                                                        closest = a1;

                                                        ErrorTerm += closest;
                                                }
                                            }

                                            if (ErrorTerm < SmallestError)
                                            {
                                                SmallestError = ErrorTerm;
                                                p1 = m1;
                                                p2 = m2;
                                                p3 = m3;
                                                p4 = m4;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Accum+=SmallestError;

                        TempPal[j][x][y][0][0] = Pal[MastX*4+x][y2][p1][0];
                        TempPal[j][x][y][0][1] = Pal[MastX*4+x][y2][p1][1];
                        TempPal[j][x][y][0][2] = Pal[MastX*4+x][y2][p1][2];

                        TempPal[j][x][y][1][0] = Pal[MastX*4+x][y2][p2][0];
                        TempPal[j][x][y][1][1] = Pal[MastX*4+x][y2][p2][1];
                        TempPal[j][x][y][1][2] = Pal[MastX*4+x][y2][p2][2];

                        TempPal[j][x][y][2][0] = Pal[MastX*4+x][y2][p3][0];
                        TempPal[j][x][y][2][1] = Pal[MastX*4+x][y2][p3][1];
                        TempPal[j][x][y][2][2] = Pal[MastX*4+x][y2][p3][2];

                        TempPal[j][x][y][3][0] = Pal[MastX*4+x][y2][p4][0];
                        TempPal[j][x][y][3][1] = Pal[MastX*4+x][y2][p4][1];
                        TempPal[j][x][y][3][2] = Pal[MastX*4+x][y2][p4][2];
                    }
                    else
                    {
                        SmallestError=0;
                        for (dy=0; dy<2; dy++)        //scan 2 rows.
                        {
                            dx2 = TileWidth[x];     //get tile-width.

                            for(dx=0; dx<dx2; dx++)    //scan column of tile.
                            {
                                cx = TileOffset[x]+MastX*80;   //get X-index into image.
                                cx += dx;                            //

                                cy = MastY*8+y*2-os+dy; //get row (Y) index.

                                switch (state)
                                {
                                case 0:    //get colour #1.

                                    TempPal[j][x][y][0][0] = pic[cx][cy][0];
                                    TempPal[j][x][y][0][1] = pic[cx][cy][1];
                                    TempPal[j][x][y][0][2] = pic[cx][cy][2];
                                    state++;
                                    break;

                                case 1:    //get colour #2.

                                    if ((pic[cx][cy][0] != TempPal[j][x][y][0][0]) ||
                                        (pic[cx][cy][1] != TempPal[j][x][y][0][1]) ||
                                        (pic[cx][cy][2] != TempPal[j][x][y][0][2]) )
                                    {
                                        TempPal[j][x][y][1][0] = pic[cx][cy][0];
                                        TempPal[j][x][y][1][1] = pic[cx][cy][1];
                                        TempPal[j][x][y][1][2] = pic[cx][cy][2];
                                        state++;
                                    }


                                    break;

                                case 2:    //get colour #3.

                                    if (((pic[cx][cy][0] != TempPal[j][x][y][0][0]) ||
                                         (pic[cx][cy][1] != TempPal[j][x][y][0][1]) ||
                                         (pic[cx][cy][2] != TempPal[j][x][y][0][2])) &&

                                        ((pic[cx][cy][0] != TempPal[j][x][y][1][0]) ||
                                         (pic[cx][cy][1] != TempPal[j][x][y][1][1]) ||
                                         (pic[cx][cy][2] != TempPal[j][x][y][1][2])))

                                    {
                                        TempPal[j][x][y][2][0] = pic[cx][cy][0];
                                        TempPal[j][x][y][2][1] = pic[cx][cy][1];
                                        TempPal[j][x][y][2][2] = pic[cx][cy][2];
                                        state++;
                                    }
                                    break;


                                case 3:    //get colour #4.

                                    if (((pic[cx][cy][0] != TempPal[j][x][y][0][0]) ||
                                         (pic[cx][cy][1] != TempPal[j][x][y][0][1]) ||
                                         (pic[cx][cy][2] != TempPal[j][x][y][0][2])) &&

                                        ((pic[cx][cy][0] != TempPal[j][x][y][1][0]) ||
                                         (pic[cx][cy][1] != TempPal[j][x][y][1][1]) ||
                                         (pic[cx][cy][2] != TempPal[j][x][y][1][2])) &&

                                        ( (pic[cx][cy][0] != TempPal[j][x][y][2][0]) ||
                                        (pic[cx][cy][1] != TempPal[j][x][y][2][1]) ||
                                        (pic[cx][cy][2] != TempPal[j][x][y][2][2])))

                                    {
                                        TempPal[j][x][y][3][0] = pic[cx][cy][0];
                                        TempPal[j][x][y][3][1] = pic[cx][cy][1];
                                        TempPal[j][x][y][3][2] = pic[cx][cy][2];
                                        state++;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if(Accum<BestSoFar)
            {
                BestSoFar=Accum;
                Line=j;
            }
        }

        for(x=0;x<4;x++)
        {
            for(y=0;y<4;y++)
            {
                IdealPal[MastX*4+x][MastY*4+y][0][0]=TempPal[Line][x][y][0][0];
                IdealPal[MastX*4+x][MastY*4+y][0][1]=TempPal[Line][x][y][0][1];
                IdealPal[MastX*4+x][MastY*4+y][0][2]=TempPal[Line][x][y][0][2];

                IdealPal[MastX*4+x][MastY*4+y][1][0]=TempPal[Line][x][y][1][0];
                IdealPal[MastX*4+x][MastY*4+y][1][1]=TempPal[Line][x][y][1][1];
                IdealPal[MastX*4+x][MastY*4+y][1][2]=TempPal[Line][x][y][1][2];

                IdealPal[MastX*4+x][MastY*4+y][2][0]=TempPal[Line][x][y][2][0];
                IdealPal[MastX*4+x][MastY*4+y][2][1]=TempPal[Line][x][y][2][1];
                IdealPal[MastX*4+x][MastY*4+y][2][2]=TempPal[Line][x][y][2][2];

                IdealPal[MastX*4+x][MastY*4+y][3][0]=TempPal[Line][x][y][3][0];
                IdealPal[MastX*4+x][MastY*4+y][3][1]=TempPal[Line][x][y][3][1];
                IdealPal[MastX*4+x][MastY*4+y][3][2]=TempPal[Line][x][y][3][2];
            }
        }

        Best[MastX][MastY]=Line;
    }
    log_progress("\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void RemapPCtoGBC( void )
{
    log_debug("RemapPCtoGBC()\n");
    u8        MastX,MastY;
    u8        Line;
    u8        width;
    u8        i,x,y,z;
    u8        gbcolor;
    u8        r,g,b;
    u32        SmallestError;
    u8        os;
    u8        y2;
    u8        sx;
    u8        dx,dy;
    u8        cx,cy;
    u32        a1,b1,c1,d1;
    u32        closest;



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

    for(MastX=0;MastX<2;MastX++)
    {
        for(MastY=0;MastY<18;MastY++)
        {
            Line=Best[MastX][MastY];
            width=0;
            for(i=0;i<4;i++)
            {
                TileOffset[i]=width;
                TileWidth[i]=SplitData[Line][i]*8;
                width+=TileWidth[i];
            }

            for(x=0;x<4;x++)
            {
                for(y=0;y<4;y++)
                {

                    SmallestError = 0x7fffffff;

                    if (MastX==0)        //add Y-offset for left of image.
                        os=1;
                    else
                        os=0;

                    y2 = (MastY*8+y*2)/2;        //setup Y-index into Pal structure.

                    sx = TileWidth[x];

                    // We now have the 4 ideal colors in our palette.
                    // Now closest color render this cell using this
                    // ideal palette.

                    for(dy=0; dy<2; dy++)        //scan 2 rows.
                    {
                        for(dx=0; dx<sx; dx++)    //scan 16/24 columns.
                        {
                            cx = TileOffset[x]+MastX*80;   //get X-index into image.
                            cx += dx;

                            cy = MastY*8+y*2-os+dy; //get row (Y) index.


                            a1=    ((IdealPal[MastX*4+x][y2][0][0]-(int)pic[cx][cy][0]) * (IdealPal[MastX*4+x][y2][0][0]-(int)pic[cx][cy][0]) ) +
                                ((IdealPal[MastX*4+x][y2][0][1]-(int)pic[cx][cy][1]) * (IdealPal[MastX*4+x][y2][0][1]-(int)pic[cx][cy][1]) ) +
                                ((IdealPal[MastX*4+x][y2][0][2]-(int)pic[cx][cy][2]) * (IdealPal[MastX*4+x][y2][0][2]-(int)pic[cx][cy][2]) ) ;

                            b1=    ((IdealPal[MastX*4+x][y2][1][0]-(int)pic[cx][cy][0]) * (IdealPal[MastX*4+x][y2][1][0]-(int)pic[cx][cy][0]) ) +
                                ((IdealPal[MastX*4+x][y2][1][1]-(int)pic[cx][cy][1]) * (IdealPal[MastX*4+x][y2][1][1]-(int)pic[cx][cy][1]) ) +
                                ((IdealPal[MastX*4+x][y2][1][2]-(int)pic[cx][cy][2]) * (IdealPal[MastX*4+x][y2][1][2]-(int)pic[cx][cy][2]) );

                            c1=    ((IdealPal[MastX*4+x][y2][2][0]-(int)pic[cx][cy][0]) * (IdealPal[MastX*4+x][y2][2][0]-(int)pic[cx][cy][0]) ) +
                                ((IdealPal[MastX*4+x][y2][2][1]-(int)pic[cx][cy][1]) * (IdealPal[MastX*4+x][y2][2][1]-(int)pic[cx][cy][1]) ) +
                                ((IdealPal[MastX*4+x][y2][2][2]-(int)pic[cx][cy][2]) * (IdealPal[MastX*4+x][y2][2][2]-(int)pic[cx][cy][2]) ) ;

                            d1=    ((IdealPal[MastX*4+x][y2][3][0]-(int)pic[cx][cy][0]) * (IdealPal[MastX*4+x][y2][3][0]-(int)pic[cx][cy][0]) ) +
                                ((IdealPal[MastX*4+x][y2][3][1]-(int)pic[cx][cy][1]) * (IdealPal[MastX*4+x][y2][3][1]-(int)pic[cx][cy][1]) ) +
                                ((IdealPal[MastX*4+x][y2][3][2]-(int)pic[cx][cy][2]) * (IdealPal[MastX*4+x][y2][3][2]-(int)pic[cx][cy][2]) ) ;


                            closest = d1;
                            gbcolor = 3;

                            if (c1 < closest)
                            {
                                closest = c1;
                                gbcolor = 2;
                            }

                            if (b1 < closest)
                            {
                                closest = b1;
                                gbcolor = 1;
                            }

                            if (a1 < closest)
                            {
                                closest = a1;
                                gbcolor = 0;
                            }

                            r = IdealPal[MastX*4+x][y2][gbcolor][0];
                            g = IdealPal[MastX*4+x][y2][gbcolor][1];
                            b = IdealPal[MastX*4+x][y2][gbcolor][2];


                            if ((MastY*8+y*2-os+dy) >= 0)
                            {
                                out[cx][MastY*8+y*2-os+dy] = gbcolor;
                                raw[0][cx][MastY*8+y*2-os+dy][0] = r;
                                raw[0][cx][MastY*8+y*2-os+dy][1] = g;
                                raw[0][cx][MastY*8+y*2-os+dy][2] = b;

                                RGBQUAD GBView=translate(raw[0][cx][MastY*8+y*2-os+dy]);

                                raw[1][cx][MastY*8+y*2-os+dy][0] = GBView.rgbRed;
                                raw[1][cx][MastY*8+y*2-os+dy][1] = GBView.rgbGreen;
                                raw[1][cx][MastY*8+y*2-os+dy][2] = GBView.rgbBlue;
                            }
                        }
                    }
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// This function will average the colours used in a square section
// of a picture, in this case 4 pixels are averaged into 1 colour.

void AddPixels (int xs, int ys, int width, int height)
{
    s32        cnt = 0;
    s32         x,y;

    avgr = avgg = avgb = 0;                    // Clear initial averages.

    for (y=0; y<height; y++)
    {
        for(x=0; x<width; x++)
        {
            avgr += pic[xs+x][ys+y][0];        //
            avgg += pic[xs+x][ys+y][1];        // Add to average.
            avgb += pic[xs+x][ys+y][2];        //
            cnt++;
        }
    }

    avgr /= cnt;                             //
    avgg /= cnt;                            // RGB average / pixels-averaged.
    avgb /= cnt;                             //
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// This function will step through every entry of a cell, which could be up to 28 pixels
// wide and return the number of unique colours used within that cell.

int CountColorsInCell(int x, int y, int sx, int os)
{
    s32        c,cx,cy,dx,dy;
    s32        count = 0;
    s32        found;
    u32        t[7*8*4][3];


    for (dy=0; dy<2; dy++)
    {
        for (dx=0; dx<sx; dx++)
        {
            cx = x + dx;     //get X-index into image.

            cy = y-os+dy;

            found = 0;

            for (c=0; c<count; c++)
            {
                if ( (pic[cx][cy][0] == t[c][0]) &&    (pic[cx][cy][1] == t[c][1]) && (pic[cx][cy][2] == t[c][2]) )
                {
                    found = 1;
                }
            }

            if (found == 0)
            {
                t[count][0] = pic[cx][cy][0];
                t[count][1] = pic[cx][cy][1];
                t[count][2] = pic[cx][cy][2];
                count++;
            }

        }
    }
        return(count);
}



// Due to the nature of the interleave on the Gameboy color all of the
// left hand side of the screen has to use the same attribute block width
// This function will determine which of the 80 combinations of attrib block
// allows the closest likeness to the original picture. The range of
// checks can be limited to within a certain range by means of the StartSplit
// and NumSplit variables

// The function returns the line in the array SplitData that has the closest match.

u8 DetermineBestLeft(u8 StartSplit, u8 NumSplit)
{
    log_debug("DetermineBestLeft()\n");
    u32        Accum;
    s8        os;
    u32        BestSoFar;
    u32        MastY;
    u32        width;
    u32        x,y,sx,sx2,sx3,y2;
    u32        i;
    u8        j;
    u32        a1,b1,c1,d1;
    u32        dx,dy;
    u32        SmallestError;
    u32        m1,m2,m3,m4;
    u32        ErrorTerm;
    u32        cx,cy;

    u32        closest;
    u32        Line=0;

      BestSoFar=0x7fffffff;
    os=1;

    for(j=StartSplit;j<(StartSplit+NumSplit);j++)                        // Total number of combinations to try
    {
         Accum=0;                                                        // Each combination has a accumalative score

        for(MastY=0;MastY<18;MastY++)                                    // Try evert attribute block on the Y Axis
        {
            log_progress(".");
            width=0;

            for(i=0;i<4;i++)                                            // There are 4 attrib blocks on the left hand side of the screen
            {
                TileOffset[i]=width;
                TileWidth[i]=SplitData[j][i]*8;
                width+=TileWidth[i];
            }

            for (x=0; x<4; x++ )                                        // There are 4 attrib blocks on the left hand side of the screen
            {
                for (y=0; y<4; y++)                                        // Each attrib block has 4 colour changes in height
                {
                    y2 = (MastY*8+y*2)/2;                                //setup Y-index into Pal structure.

                    sx = TileOffset[x];
                    sx2 = sx + TileWidth[x];

                    for (sx3=0;sx<sx2;sx+=2 )
                    {
                        AddPixels(sx,MastY*8+y*2,2,2);                    // Get an average for the pixels in an attrib block

                        Pal[x][y2][sx3][0] = (unsigned char)avgr;        // There could be up to 28 seperate palettes per block
                        Pal[x][y2][sx3][1] = (unsigned char)avgg;
                        Pal[x][y2][sx3][2] = (unsigned char)avgb;
                        sx3++;
                    }
                }
            }

            for (x=0; x<4; x++ )                                        // Step through the 4 attribute blocks for the left side of screen
            {
                for (y=0; y<4; y++)                                        // Step through the 4 colour changes per attrib block
                {
                    SmallestError = 0x7fffffff;                            // SmallestError is the one that gets through

                    y2 = (MastY*8+y*2)/2;                                // Setup Y-index into Pal structure.

                    sx = TileWidth[x];
                    if ( CountColorsInCell(TileOffset[x],MastY*8+y*2,sx,os) > 4 )    // Just how many colours are in this section, ignore if less than 4 colours
                    {
                        sx = sx/2;

                        for(m4=0; m4<sx; m4++)                                        // Try all combinations to find lowest error-rate.
                        {
                            for(m3=0; m3<sx; m3++)
                            {
                                for(m2=0; m2<sx; m2++)
                                {
                                    for(m1=0; m1<sx; m1++)
                                    {
                                        if ( (m4 > m3) && (m3 > m2) && (m2 > m1) )    // Avoid duplicate checks.
                                        {
                                            ErrorTerm = 0;                            // Clear total error.

                                            for(dy=0; dy<2; dy++)                    // Scan 2 rows.
                                            {
                                                for(dx=0; dx<TileWidth[x]; dx++)    // Scan column of tile.
                                                {
                    //log_debug("     * dx=%d,dy=%d\n",dx, dy);
                                                    cx = TileOffset[x] + dx;        // Get X-index into image.

                                                    cy = MastY*8+y*2-os+dy;            // Get row (Y) index.

                                                    if(cy<0) cy=0;

                                                    a1= (Pal[x][y2][m1][0]-(int)pic[cx][cy][0]) * (Pal[x][y2][m1][0]-(int)pic[cx][cy][0]) +        // Calc RGB Distance
                                                        (Pal[x][y2][m1][1]-(int)pic[cx][cy][1]) * (Pal[x][y2][m1][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[x][y2][m1][2]-(int)pic[cx][cy][2]) * (Pal[x][y2][m1][2]-(int)pic[cx][cy][2]) ;
                                                    b1= (Pal[x][y2][m2][0]-(int)pic[cx][cy][0]) * (Pal[x][y2][m2][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[x][y2][m2][1]-(int)pic[cx][cy][1]) * (Pal[x][y2][m2][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[x][y2][m2][2]-(int)pic[cx][cy][2]) * (Pal[x][y2][m2][2]-(int)pic[cx][cy][2]) ;

                                                    c1= (Pal[x][y2][m3][0]-(int)pic[cx][cy][0]) * (Pal[x][y2][m3][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[x][y2][m3][1]-(int)pic[cx][cy][1]) * (Pal[x][y2][m3][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[x][y2][m3][2]-(int)pic[cx][cy][2]) * (Pal[x][y2][m3][2]-(int)pic[cx][cy][2]) ;

                                                    d1= (Pal[x][y2][m4][0]-(int)pic[cx][cy][0]) * (Pal[x][y2][m4][0]-(int)pic[cx][cy][0]) +
                                                        (Pal[x][y2][m4][1]-(int)pic[cx][cy][1]) * (Pal[x][y2][m4][1]-(int)pic[cx][cy][1]) +
                                                        (Pal[x][y2][m4][2]-(int)pic[cx][cy][2]) * (Pal[x][y2][m4][2]-(int)pic[cx][cy][2]) ;

                                                    closest = d1;

                                                    if (c1 < closest)                // Determine which palette is closest
                                                        closest = c1;

                                                    if (b1 < closest)
                                                        closest = b1;

                                                    if (a1 < closest)
                                                        closest = a1;

                                                    ErrorTerm += closest;
                                                }
                                            }

                                            if (ErrorTerm < SmallestError)
                                                SmallestError = ErrorTerm;            // Store details of smallest distance
                                        }
                                    }
                                }
                            }
                        }
                        Accum=Accum+SmallestError;        // Accumulate total for every attribute block
                    }
                }
            }
        }

        if(Accum<BestSoFar)        // Is this the best line yet?
        {
            BestSoFar=Accum;
            Line=j;                // Store the position of the best fit.
        }
    }
    log_progress("\n");
    return Line;
}

