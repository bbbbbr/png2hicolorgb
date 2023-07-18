#ifndef __hicolour_h__
#define __hicolour_h__


extern	HWND			Ghdwnd;								// Global window handle
extern	u8				TileOffset[4];						// Offset into screen for attribute start
extern	u8				TileWidth[4];						// No of character attributes width
extern	u8				SplitData[80][4];
extern	u8				Pal[8][72][28][3];					// Palettes for every other line
extern	u8				pic[160][144][3];					// Original Picture
extern	u8				IdealPal[8][72][4][3];				// The best fit palette
extern	u8				Best[2][18];						// Best Attribute type to use
extern	u8				AttribTable[18][20];				// Attribute table for final render
extern	u8				out[160][144];						// Output data
extern	u8				raw[2][160][144][3];				// Original Picture Raw format
extern	RGBQUAD			GBView;




int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);
void AbortConvert(BOOL OK);
DWORD WINAPI Convert_ThreadFunc(LPVOID lpParameter);
void Convert(HWND hdwnd);
void ExitConvert(void);
BOOL CALLBACK DialogFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ExportTileSet(HWND hWnd, char* prefix);
void ExportPalettes(HWND hWnd, char* prefix);
void ExportAttrMap(HWND hWnd, char* prefix);
int CheckTGA(void);
void Method4(void);
RGBQUAD translate(BYTE rgb[3]);
unsigned int ImageRating(u8 *src, u8 *dest, int StartX, int StartY, int Width, int Height);
void DoOtherConversion(int ConvertType);
int Method1(int StartX, int Width, int StartY, int Height, int StartJ, int FinishJ, int ConvertType);










RGBQUAD translate(BYTE rgb[3]);


#endif
