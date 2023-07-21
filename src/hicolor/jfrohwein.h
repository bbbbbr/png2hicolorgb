#ifndef __jfrohwein_h__
#define __jfrohwein_h__


// #include <windows.h>
// #include <winuser.h>
// #include <commctrl.h>
#include <stdio.h>

void	RemapGB(u8 MastX, u8 StartSplit, u8 NumSplit);
void	RemapPCtoGBC( void );
void	AddPixels (int xs, int ys, int width, int height);
int		CountColorsInCell(int x, int y, int sx, int os);
u8		DetermineBestLeft(u8 StartSplit, u8 NumSplit);


#endif
