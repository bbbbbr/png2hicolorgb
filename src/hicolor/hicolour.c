#include <windows.h>
#include <winuser.h>
#include <commctrl.h>
#include <stdio.h>

#include "defines.h"
#include "resource.h"
#include "hicolour.h"
#include "jfrohwein.h"
#include "median.h"
#include "wu.h"

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


// V1.0	- 27th March 2000 - First public release
// V1.1 - 30th March 2000 - Rob Jones added seperate thread for conversion process
// V1.2 - 8th April 2000 - Added other quantisation methods


/*		To Do - When I get around to it ;)

  Batch conversion
  tga output for comparison purposes
  Other quantisation methods ?
  Outputing tiles / attributes based on predefined map file
  Tidy the code up - it's getting a bit sloppy
  Speed up the code
*/



// Function prototypes



int	br,bg,bb;

u8		SplitData[80][4];


typedef struct
{
	u8		p1;
	u8		p2;
	u8		FileType;
	u8		p3[9];
	u16		XSize;
	u16		YSize;
	u8		BitDepth;
	u8		c1;
	u8		data[160*144][3];
}TGA_TYPE;



u8				QR[144][160][3];
u8				TileOffset[4];						// Offset into screen for attribute start
u8				TileWidth[4];						// No of character attributes width
u8				Pal[8][72][28][3];					// Palettes for every other line
u8				IdealPal[8][72][4][3];				// The best fit palette
u8				pic[160][144][3];					// Original Picture
u8				pic2[160][144][3];					// Output picture
u8				out[160][144];						// Output data
u8				raw[2][160][144][3];				// Original Picture Raw format
u8				Best[2][18];						// Best Attribute type to use
s32				GWeight=100;						// Colour weighting Green
s32				RWeight=100;						// Colour weighting Red
s32				BWeight=100;						// Colour weighting Blue
u8				GotFile=0;							// Is there a file loaded
u8				LConversion;						// Conversion type for left hand side of the screen
u8				RConversion;						// Conversion type for right hand side of the screen
HWND			Ghdwnd;								// Global window handle
u8				AttribTable[18][20];				// Attribute table for final render
s32				ViewType=0;							// View type 0 = Normal , 1 = GB Color
u8				OldLConv=0;							// Conversion type
u8				OldRConv=0;
PBYTE			pBuffer;
u8				Message[2000];
s32				ConvertType=2;
s32				OldConvType=-1;

u8				Data[160*144*3];

u32				TempD;
s32				BestLine=0;
u32				BestQuantLine;
RGBQUAD			GBView;



static	BYTE 	*pBitsdest;	

BYTE			*pBitssource;
HANDLE 			ConvertThread=NULL;
DWORD			ConvertThreadID;


#define MAX_CONVERSION_TYPES	83

TCHAR	*ListStyle[]=
{
	"Adaptive 1","Adaptive 2","Adaptive 3",
	"3-2-3-2","2-3-2-3","2-2-3-3","2-3-3-2","3-2-2-3","3-3-2-2","4-2-2-2","2-2-2-4",
	"2-2-4-2","2-4-2-2","1-1-2-6","1-1-3-5","1-1-4-4","1-1-5-3","1-1-6-2","1-2-1-6",
	"1-2-2-5","1-2-3-4","1-2-4-3","1-2-5-2","1-2-6-1","1-3-1-5","1-3-2-4","1-3-3-3",
	"1-3-4-2","1-3-5-1","1-4-1-4","1-4-2-3","1-4-3-2","1-4-4-1","1-5-1-3","1-5-2-2",
	"1-5-3-1","1-6-1-2","1-6-2-1","2-1-1-6","2-1-2-5","2-1-3-4","2,1,4,3","2-1-5-2",
	"2-1-6-1","2-2-1-5","2-2-5-1","2-3-1-4","2-3-4-1","2-4-1-3","2-4-3-1","2-5-1-2",
	"2-5-2-1","2-6-1-1","3-1-1-5","3-1-2-4","3-1-3-3","3-1-4-2","3-1-5-1","3-2-1-4",
	"3-2-4-1","3-3-1-3","3-3-3-1","3-4-1-2","3-4-2-1","3-5-1-1","4-1-1-4","4-1-2-3",
	"4-1-3-2","4-1-4-1","4-2-1-3","4-2-3-1","4-3-1-2","4-3-2-1","4-4-1-1","5-1-1-3",
	"5-1-2-2","5-1-3-1","5-2-1-2","5-2-2-1","5-3-1-1","6-1-1-2","6-1-2-1","6-2-1-1"
};


#define MAX_QUANTISER_TYPES		4

TCHAR	*ConvertStyle[]=
{
	"Original Method (J.Frohwein)",
	"Median Cut - No Dither",
	"Median Cut - With Dither",
	"Wu Quantiser"
};


#pragma comment(lib,"comctl32.lib")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows Main Function
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	InitCommonControls();
	DialogBox(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,DialogFunc);
	return 0;
}



void AbortConvert(BOOL OK)
{
	InvalidateRect(Ghdwnd,NULL,FALSE);
	EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTLEFT),TRUE);
	EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTTYPE),TRUE);
	EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTRIGHT),TRUE);
	EnableWindow(GetDlgItem(Ghdwnd,IDC_FILE),TRUE);
	SendMessage(GetDlgItem(Ghdwnd,IDC_CONVERT),WM_SETTEXT,0,(LPARAM)"&Convert");
	if (OK) 
	{
		// Conversion is OK, so enable saving/viewing buttons
		EnableWindow(GetDlgItem(Ghdwnd,IDC_SAVE),TRUE);
		EnableWindow(GetDlgItem(Ghdwnd,IDC_VIEW_ATTRIBS),TRUE);
	}
}


DWORD WINAPI Convert_ThreadFunc(LPVOID lpParameter)
{
	s32		x,y,z;
	u8		col;

	if(ConvertType==0)
	{
		Method4();
 
		for(y=0;y<144;y++)
		{
			for(x=0;x<160;x++)
			{
				col=Picture256[y*160+x];
				for(z=0;z<3;z++)
				{
					*(pBitsdest+(143-y)*3*160+x*3+z)=raw[ViewType][x][y][2-z];
				}
			}
		}
	}
	else
	{
		DoOtherConversion(ConvertType-1);
	}


	AbortConvert(TRUE);

	CloseHandle(ConvertThread);
	ConvertThread = NULL;

	return(TRUE);
}


void Convert(HWND hdwnd)
{
	if (ConvertThread) 
	{
		CloseHandle(ConvertThread);
		ConvertThread = NULL;
	}

	// Create and run thread to convert image
	ConvertThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)Convert_ThreadFunc,(LPVOID)0,(DWORD)NULL,(LPDWORD)&ConvertThreadID);
}

void ExitConvert(void)
{
	// Exit any running thread now!
	if (ConvertThread) 
	{
		TerminateThread(ConvertThread,FALSE);
		CloseHandle(ConvertThread);
		ConvertThread = NULL;
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows function (dialog)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK DialogFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static	OPENFILENAME	ofn;
	static	TCHAR			szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static	TCHAR			szLFilter[]=	TEXT("Targa Files (*.tga)\0*.tga\0");
	//										TEXT("Bitmap Files (*.bmp)\0*.bmp\0\0");
	static	TCHAR			szSFilter[]=TEXT("Tile Files (*.til)\0*.til\0");

	static	BITMAPINFOHEADER	bmihsource;
	static	HBITMAP				hBitmapsource;
	static	BITMAP				bitmapsource;

	static	BITMAPINFOHEADER	bmihdest;
	static	HBITMAP				hBitmapdest;
	static	BITMAP				bitmapdest;

	HDC							hdc,hdcMem;
	s32							i;
	s32							bp,pl;
	HANDLE						hFile;
	s32							iFileLength;
	DWORD						dwBytesRead;
	u8							TmpBuff[100];
	u8							lb,rb;
	s32							x,y,z;

	Ghdwnd=hdwnd;

	switch(message)
	{

		case WM_INITDIALOG:


			ofn.lStructSize			= sizeof(OPENFILENAME);									// Set up file selector structure
			ofn.hwndOwner			= hdwnd;
			ofn.hInstance			= NULL;
			ofn.lpstrCustomFilter	= NULL;
			ofn.nMaxCustFilter		= 0;
			ofn.nFilterIndex		= 0;
			ofn.nMaxFile			= MAX_PATH;
			ofn.nMaxFileTitle		= MAX_PATH;
			ofn.lpstrInitialDir		= NULL;
			ofn.lCustData			= 0;
			ofn.lpfnHook			= NULL;
			ofn.lpTemplateName		= NULL;
	
	
			SendDlgItemMessage(hdwnd,IDC_SLIDER1,TBM_SETRANGE,TRUE,MAKELONG(0,50));			// Set up sliders for RGB
			SendDlgItemMessage(hdwnd,IDC_SLIDER1,TBM_SETPOS,1,25);
	
			SendDlgItemMessage(hdwnd,IDC_SLIDER2,TBM_SETRANGE,TRUE,MAKELONG(0,50));
			SendDlgItemMessage(hdwnd,IDC_SLIDER2,TBM_SETPOS,1,25);
	
			SendDlgItemMessage(hdwnd,IDC_SLIDER3,TBM_SETRANGE,TRUE,MAKELONG(0,50));
			SendDlgItemMessage(hdwnd,IDC_SLIDER3,TBM_SETPOS,1,25);
	
			ViewType=0;				// View = True RGB
	
			LConversion=3;			// Default Conversion (Adaptive 3) Left Screen
			RConversion=3;			// Default Conversion (Adaptive 3) Righ Screen
	
	
			for(i=0;i<MAX_CONVERSION_TYPES;i++)													// Send total list of conversion type to combo boxes
			{
				SendDlgItemMessage(hdwnd,IDC_CONVERTLEFT,CB_ADDSTRING,0,(LPARAM)ListStyle[i]);
				SendDlgItemMessage(hdwnd,IDC_CONVERTRIGHT,CB_ADDSTRING,0,(LPARAM)ListStyle[i]);
			}

			
			for(i=0;i<MAX_QUANTISER_TYPES;i++)													// Send total list of conversion type to combo boxes
				SendDlgItemMessage(hdwnd,IDC_CONVERTTYPE,CB_ADDSTRING,0,(LPARAM)ConvertStyle[i]);
	


			SendDlgItemMessage(hdwnd,IDC_CONVERTLEFT,CB_SETCURSEL,LConversion,0);					// Set default convert type for left screen
			SendDlgItemMessage(hdwnd,IDC_CONVERTRIGHT,CB_SETCURSEL,RConversion,0);					// Set default convert type for right screen
	
			SendDlgItemMessage(hdwnd,IDC_CONVERTTYPE,CB_SETCURSEL,ConvertType,0);					// Set default convert type for right screen
	
	
			bmihsource.biSize			= sizeof(BITMAPINFOHEADER);									// Set up Dib pictures for input / output
			bmihsource.biWidth			= 160;
			bmihsource.biHeight			= 144;
			bmihsource.biPlanes			= 1;
			bmihsource.biBitCount		= 24;
			bmihsource.biCompression	= BI_RGB;
			bmihsource.biSizeImage		= 0;
			bmihsource.biXPelsPerMeter	= 0;
			bmihsource.biYPelsPerMeter	= 0;
			bmihsource.biClrUsed		= 0;
			bmihsource.biClrImportant	= 0;
	
			hBitmapsource = CreateDIBSection(NULL,(BITMAPINFO *)&bmihsource, 0, &pBitssource, NULL, 0);
	
			bmihdest.biSize			= sizeof(BITMAPINFOHEADER);
			bmihdest.biWidth		= 160;
			bmihdest.biHeight		= 144;
			bmihdest.biPlanes		= 1;
			bmihdest.biBitCount		= 24;
			bmihdest.biCompression	= BI_RGB;
			bmihdest.biSizeImage	= 0;
			bmihdest.biXPelsPerMeter= 0;
			bmihdest.biYPelsPerMeter= 0;
			bmihdest.biClrUsed		= 0;
			bmihdest.biClrImportant = 0;
	
			hBitmapdest = CreateDIBSection(NULL,(BITMAPINFO *)&bmihdest, 0, &pBitsdest, NULL, 0);
	
			hdc=GetDC(hdwnd);						// Check for current display type
			bp=GetDeviceCaps(hdc,BITSPIXEL);
			pl=GetDeviceCaps(hdc,PLANES);
			ReleaseDC(hdwnd,hdc);
			if(pl!=1 || bp<16)			
				MessageBox(hdwnd,"To view the results of the conversion - Please ensure you use at least a 16 bit display depth","Gameboy HiColour Convertor",MB_OK);
	
			return TRUE;



		case WM_PAINT:

			GetObject(hBitmapsource,sizeof(BITMAP),&bitmapsource);
			hdc=GetDC(hdwnd);
			hdcMem=CreateCompatibleDC(hdc);
			SelectObject(hdcMem,hBitmapsource);
			BitBlt(hdc,21,26,bitmapsource.bmWidth,bitmapsource.bmHeight,hdcMem,0,0,SRCCOPY);
			DeleteDC(hdcMem);
			ReleaseDC(hdwnd,hdc);
	
			GetObject(hBitmapdest,sizeof(BITMAP),&bitmapdest);
			hdc=GetDC(hdwnd);
			hdcMem=CreateCompatibleDC(hdc);
			SelectObject(hdcMem,hBitmapdest);
			BitBlt(hdc,21,200,bitmapdest.bmWidth,bitmapdest.bmHeight,hdcMem,0,0,SRCCOPY);
			DeleteDC(hdcMem);
			ReleaseDC(hdwnd,hdc);
	
			return FALSE;			// Dont tell windows we have done anything - naughty ??


		case WM_CLOSE:

			if(MessageBox(hdwnd,"Are you sure you want to Quit","Gameboy HiColour Convertor",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
				EndDialog(hdwnd,0);

			return TRUE;


		case WM_COMMAND:

			switch(LOWORD(wParam))
			{
		
				case IDC_EXIT:
		
					if(MessageBox(hdwnd,"Are you sure you want to Quit","Gameboy HiColour Convertor",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
						EndDialog(hdwnd,0);
					return TRUE;
		
		
		
				case IDC_FILE:
		
					ofn.lpstrFilter			= szLFilter;
					ofn.lpstrFile			= szFileName;
					ofn.lpstrFileTitle		= szTitleName;
					ofn.lpstrTitle			= "HiColour Image for Conversion\0";
					ofn.Flags				= OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
					ofn.nFileOffset			= 0;
					ofn.nFileExtension		= 0;
					ofn.lpstrDefExt			= TEXT("tga");
					*(szFileName)			= 0;
		
					if(!GetOpenFileName(&ofn))				// Get the filename
						return FALSE;
		
					if(INVALID_HANDLE_VALUE==(hFile=CreateFile(szFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)))
						return FALSE;
		
					iFileLength = GetFileSize(hFile,NULL);	// Length of file
		
					pBuffer = (PBYTE)malloc(iFileLength);	// Allocate memory for file
			
					ReadFile(hFile,pBuffer,iFileLength,&dwBytesRead,NULL);		// Read file into memory
		
					CloseHandle(hFile);			// Close file
		
		
					if(CheckTGA()==0)		// Valid File
					{
						for(y=0;y<144;y++)
						{
							for(x=0;x<160;x++)
							{
								for(z=0;z<3;z++)
								{
									*(pBitssource+(143-y)*3*160+x*3+z)=pic2[x][y][2-z];			// Invert the dib, cos windows likes it like that !!
								}
							}
						}

						EnableWindow(GetDlgItem(hdwnd,IDC_SAVE),FALSE);
						EnableWindow(GetDlgItem(hdwnd,IDC_VIEW_ATTRIBS),FALSE);
						EnableWindow(GetDlgItem(hdwnd,IDC_CONVERT),TRUE);			// Enable CONVERT button
		
						InvalidateRect(hdwnd,NULL,FALSE);							// Tell windows to redraw the screen
					}
		
					return TRUE;
		
		
		
				case IDC_SAVE:
		
					strcpy(szFileName,"test");
		
					ofn.lpstrFilter			= szSFilter;
					ofn.lpstrFile			= szFileName;
					ofn.lpstrFileTitle		= szTitleName;
					ofn.lpstrTitle			= "HiColour Image Code and Data File Name\0";
					ofn.Flags				= OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
					ofn.nFileOffset			= 0;
					ofn.nFileExtension		= 0;
					ofn.lpstrDefExt			= TEXT("");
		
					if(!GetSaveFileName(&ofn))
						return FALSE;
		
					i = 0;
		
					while(szFileName[i] != '.')
						i++;
		
					szFileName[i] = 0;
					ExportTileSet(hdwnd, szFileName);
					ExportPalettes(hdwnd, szFileName);
					ExportAttrMap(hdwnd, szFileName);
		
					return TRUE;
		
		
		
				case IDC_EXPORT:
		
					if(SendDlgItemMessage(hdwnd,IDC_EXPORT,BM_GETCHECK,0,0)==BST_UNCHECKED)
					{
						SendDlgItemMessage(hdwnd,IDC_EXPORT,BM_SETCHECK,BST_CHECKED,0);
					}
					else
					{
						SendDlgItemMessage(hdwnd,IDC_EXPORT,BM_SETCHECK,BST_UNCHECKED,0);
					}
		
					return TRUE;
		
		
		
				case IDC_CONVERTLEFT:
		
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						LConversion=(u8)SendDlgItemMessage(hdwnd,IDC_CONVERTLEFT,CB_GETCURSEL,0,0);
						if(LConversion!=OldLConv)
						{
							EnableWindow(GetDlgItem(hdwnd,IDC_SAVE),FALSE);
							EnableWindow(GetDlgItem(hdwnd,IDC_VIEW_ATTRIBS),FALSE);
							OldLConv=LConversion;
						}
		
						if(LConversion<3||RConversion<3)
						{
							SendMessage(GetDlgItem(hdwnd,IDC_EXPORT),BM_SETCHECK,BST_INDETERMINATE,0);				// Set check box for export attribute tables
							EnableWindow(GetDlgItem(hdwnd,IDC_EXPORT),FALSE);
						}
						else
						{
							EnableWindow(GetDlgItem(hdwnd,IDC_EXPORT),TRUE);
							if(SendDlgItemMessage(hdwnd,IDC_EXPORT,BM_GETCHECK,0,0)==BST_INDETERMINATE)
								SendMessage(GetDlgItem(hdwnd,IDC_EXPORT),BM_SETCHECK,BST_CHECKED,0);				// Set check box for export attribute tables
						}
		
						if(GotFile==1)
							EnableWindow(GetDlgItem(hdwnd,IDC_CONVERT),TRUE);
					}
		
					return TRUE;
		
		
		
				case IDC_CONVERTRIGHT:
		
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						RConversion=(u8)SendDlgItemMessage(hdwnd,IDC_CONVERTRIGHT,CB_GETCURSEL,0,0);
						if(RConversion!=OldRConv)
						{
							EnableWindow(GetDlgItem(hdwnd,IDC_SAVE),FALSE);
							EnableWindow(GetDlgItem(hdwnd,IDC_VIEW_ATTRIBS),FALSE);
							OldRConv=RConversion;
						}
		
						if(LConversion<3||RConversion<3)
						{
							SendMessage(GetDlgItem(hdwnd,IDC_EXPORT),BM_SETCHECK,BST_INDETERMINATE,0);				// Set check box for export attribute tables
							EnableWindow(GetDlgItem(hdwnd,IDC_EXPORT),FALSE);
						}
						else
						{
							EnableWindow(GetDlgItem(hdwnd,IDC_EXPORT),TRUE);
							if(SendDlgItemMessage(hdwnd,IDC_EXPORT,BM_GETCHECK,0,0)==BST_INDETERMINATE)
								SendMessage(GetDlgItem(hdwnd,IDC_EXPORT),BM_SETCHECK,BST_CHECKED,0);				// Set check box for export attribute tables
						}
		
						if(GotFile==1)
							EnableWindow(GetDlgItem(hdwnd,IDC_CONVERT),TRUE);
					}
		
					return TRUE;
		
		
				case IDC_CONVERTTYPE:
		
					if(HIWORD(wParam)==CBN_SELCHANGE)
					{
						ConvertType=(u8)SendDlgItemMessage(hdwnd,IDC_CONVERTTYPE,CB_GETCURSEL,0,0);
						if(ConvertType!=OldConvType)
						{
							EnableWindow(GetDlgItem(hdwnd,IDC_SAVE),FALSE);
							EnableWindow(GetDlgItem(hdwnd,IDC_VIEW_ATTRIBS),FALSE);
							OldConvType=ConvertType;
						}
		
					}
					return TRUE;


		
				case IDC_CONVERT:

					if (ConvertThread) 
					{	// Cancel?
						ExitConvert();
						AbortConvert(FALSE);
					}
					else 
					{				
						SendMessage(GetDlgItem(hdwnd,IDC_CONVERT),WM_SETTEXT,0,(LPARAM)"&Cancel");
						EnableWindow(GetDlgItem(Ghdwnd,IDC_FILE),FALSE);		
						EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTLEFT),FALSE);		
						EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTRIGHT),FALSE);
						EnableWindow(GetDlgItem(Ghdwnd,IDC_CONVERTTYPE),FALSE);
						EnableWindow(GetDlgItem(Ghdwnd,IDC_SAVE),FALSE);
						EnableWindow(GetDlgItem(Ghdwnd,IDC_VIEW_ATTRIBS),FALSE);
			
						RWeight=SendDlgItemMessage(hdwnd,IDC_SLIDER1,TBM_GETPOS,0,0)*4+1;
						GWeight=SendDlgItemMessage(hdwnd,IDC_SLIDER2,TBM_GETPOS,0,0)*4+1;
						BWeight=SendDlgItemMessage(hdwnd,IDC_SLIDER3,TBM_GETPOS,0,0)*4+1;
			
						for(x=0;x<160;x++)
						{
							for(y=0;y<144;y++)
							{
								pic[x][y][0]=(u8)(pic2[x][y][0]*RWeight/100);
								pic[x][y][1]=(u8)(pic2[x][y][1]*GWeight/100);
								pic[x][y][2]=(u8)(pic2[x][y][2]*BWeight/100);
								for(i=0;i<3;i++)
								{
									if(pic[x][y][i]>255)
										pic[x][y][i]=255;

									*(Data+y*160*3+x*3+i)=pic[x][y][i];
								}
							}
						}
			
	  					Convert(hdwnd);
					}
		
					return TRUE;
		
		
		
				case IDC_INFO:
		
					MessageBox(hdwnd,"Version 1.2 beta\n\n\nOriginal Concept : Icarus Productions\n\nOriginal Code : Jeff Frohwein\n\nFull Screen Modification : Anon\n\nAdaptive Code : Glen Cook\n\nWindows Interface : Glen Cook\n\nAdditional Windows Programming : Rob Jones\n\nOriginal Quantiser Code : Benny\n\nQuantiser Conversion : Glen Cook\n\nComments to GlenCook@hotmail.com\n","Gameboy HiColour Convertor",MB_OK|MB_ICONINFORMATION);
					return TRUE;
		
		
		
				case IDC_TRUE_GBC:
		
					if(SendDlgItemMessage(hdwnd,IDC_TRUE_GBC,BM_GETCHECK,0,0)==BST_CHECKED)
						ViewType=1;
					else
						ViewType=0;
		
					for(y=0;y<144;y++)
					{
						for(x=0;x<160;x++)
						{
							for(z=0;z<3;z++)
							{
								*(pBitsdest+(143-y)*3*160+x*3+z)=raw[ViewType][x][y][2-z];
							}
						}
					}
		
					InvalidateRect(hdwnd,NULL,FALSE);
		
					return TRUE;


				case IDC_VIEW_ATTRIBS:
		
					Message[0]=0;
				
					for(y=0;y<18;y++)
					{
						lb=Best[0][y];
						rb=Best[1][y];
						sprintf(TmpBuff," %d - %d - %d - %d : %d - %d - %d - %d\n\0",	SplitData[lb][0],SplitData[lb][1],SplitData[lb][2],SplitData[lb][3],
																						SplitData[rb][0],SplitData[rb][1],SplitData[rb][2],SplitData[rb][3]);

						strcat(Message,TmpBuff);
					}
				
					MessageBox(Ghdwnd,Message,"Attribute Map",MB_OK);

					return TRUE;

			}
	}

	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ExportTileSet(HWND hWnd, char* prefix)
{
	char filename0[MAX_PATH*2];

	HANDLE	hFile0;

	DWORD	byteWritten;
	u32		x, y;
	u8		c1,c2;
	u8		dx,dy;
	u8		c;


	strcpy(filename0, prefix);
	strcat(filename0, ".TIL");

	SetFileAttributes(filename0, FILE_ATTRIBUTE_NORMAL);

	hFile0 = CreateFile(filename0, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile0 == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd,"Unable to create file to export","Gameboy HiColour Convertor", MB_OK);
		return ;
	}

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

	   			WriteFile(hFile0,&c2,1,&byteWritten,NULL);
	   			WriteFile(hFile0,&c1,1,&byteWritten,NULL);
			}
		}
	}
	
	CloseHandle(hFile0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ExportPalettes(HWND hWnd, char* prefix)
{
	char filename[MAX_PATH * 2];

	HANDLE	hFile;

	DWORD	byteWritten;
	BYTE	tmpByte;
	s32		i, j, k;
	s32		r,g,b,v;

	strcpy(filename, prefix);
	strcat(filename, ".pal");	

	SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);

	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd,"Unable to create file to export","Gameboy HiColour Convertor", MB_OK);
		return ;
	}


	for (i = 0; i < 72*2; i++) // Number of palette sets
	{
		for (j = 0; j < 4; j++) // each palette in the set
		{	
			for(k=0; k<4;k++)
			{
				r = IdealPal[(i%2)*4+j][i/2][k][0];
				g = IdealPal[(i%2)*4+j][i/2][k][1];
				b = IdealPal[(i%2)*4+j][i/2][k][2];
					
				v = ((b/8)*32*32) + ((g/8)*32) + (r/8);
	
				tmpByte=(u8)(v&255);
				WriteFile(hFile, &tmpByte, 1, &byteWritten, NULL);
	
				tmpByte=(u8)(v/256);
				WriteFile(hFile, &tmpByte, 1, &byteWritten, NULL);
			}

		}
	}

	tmpByte = 0x2d;

	WriteFile(hFile, &tmpByte, 1, &byteWritten, NULL);

	CloseHandle(hFile);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void ExportAttrMap(HWND hWnd, char* prefix)
{
	char	filename[MAX_PATH*2];

	HANDLE	hFile;

	DWORD	byteWritten;
	s32		i, x, y;
	BYTE	buf, pal;

	strcpy(filename, prefix);
	strcat(filename, ".MAP"); 
	
	SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);

	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd,"Unable to create file to export","Gameboy HiColour Convertor", MB_OK);
		return ;
	}

	for (i = 0; i < 360; i++)
	{
		buf = (u8)(((BYTE) i < 128) ? ((BYTE)i) + 128 : ((BYTE)i) - 128);
		WriteFile(hFile, &buf, 1, &byteWritten, NULL);
	}

	CloseHandle(hFile);

	strcpy(filename, prefix);
	strcat(filename, ".ATR");

	SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL);

	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(hWnd,"Unable to create file to export","Gameboy HiColour Convertor", MB_OK);
		return ;
	}

	i = 0;

	for (y = 0; y < 18; y++)
	{
		for (x = 0; x < 20; x++)
		{
			pal = (BYTE)AttribTable[y][x];
			buf = (u8)((i<256) ?  pal :pal|0x08);
			i++;
			WriteFile(hFile,&buf,1,&byteWritten,NULL);
		}
	}

	CloseHandle(hFile);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int CheckTGA(void)
{
	TGA_TYPE	*pTGA;
	s32			yflip=0;
	s32			x,y,y1;
	u8			r,g,b;
	s32			count;


	pTGA=(TGA_TYPE *)pBuffer;


	if (pTGA->c1 & 0x20) yflip = 1;

	if ( (pTGA->FileType != 2) || (pTGA->XSize != 160) || (pTGA->YSize != 144) || (pTGA->BitDepth != 24) )
	{
		if (pTGA->FileType != 2)
		{
			if (pTGA->FileType == 10)
			{
				MessageBox(Ghdwnd,"TGA MUST be uncompressed","Gameboy HiColour Convertor",MB_ICONWARNING);
			}
			else
			{
				MessageBox(Ghdwnd,"This is not a TGA Format file","Gameboy HiColour Convertor",MB_ICONWARNING);
			}
		}
		else
		{
			if (pTGA->XSize != 160)
				MessageBox(Ghdwnd,"X width of tga file must be 160","Gameboy HiColour Convertor",MB_ICONWARNING);

			if (pTGA->YSize != 144)
				MessageBox(Ghdwnd,"Y height of tga file must be 144","Gameboy HiColour Convertor",MB_ICONWARNING);

			if (pTGA->BitDepth != 24)
				MessageBox(Ghdwnd,"Color depth of file must be 24 bit","Gameboy HiColour Convertor",MB_ICONWARNING);
		}

		return 1;
	}

	count=0;

	for (y=0; y<pTGA->YSize; y++)
	{
		for (x=0; x<pTGA->XSize; x++)
		{
			y1 = pTGA->YSize-1-y;

			b=pTGA->data[count][0];
			g=pTGA->data[count][1];
			r=pTGA->data[count++][2];
			if (yflip)
				y1 = y;

			pic2[x][y1][0] = (u8)(r & 0xf8);
			pic2[x][y1][1] = (u8)(g & 0xf8);
			pic2[x][y1][2] = (u8)(b & 0xf8);
		}
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Method4(void)
{

	u8				StartSplit=0;
	u8				NumSplit=1;
	u16				Steps=1;
	u8				res;


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



	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETPOS,0,0);
	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, Steps));
	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETSTEP,1,0);


	res=DetermineBestLeft(StartSplit,NumSplit);


	RemapGB(0,res,1);

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

	RemapGB(1,StartSplit,NumSplit);
	RemapPCtoGBC();


}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// This section of code is used to convert an RGB (pc) triplet into a RGB (gameboy)
// triplet. This section of code was kindly donated by Brett Bibby (GameBrains).

BYTE intensity[32] = 
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

RGBQUAD translate(BYTE rgb[3])
{
	RGBQUAD color;
	BYTE	tmp[3];
	BYTE	m[3][3];
	BYTE	i,j;

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

	color.rgbRed	= tmp[0];
	color.rgbGreen	= tmp[1];
	color.rgbBlue	= tmp[2];

	return color;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Data table containing all of the possible combinations of attribute blocks
// for one side of the screen.

// The higher the adaptive level, the more combinations of attributes are tested.

u8	SplitData[80][4]=
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
	unsigned int	tot;
	int				x,y;
	unsigned int	accum=0;
	int				scradd;

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


void DoOtherConversion(int ConvertType)
{

	int		res;
	int		x,y,z,i;
	int		StartSplit=0;
	int		NumSplit=1;
	int		Steps;
	int		MastX,MastY;
	int		Line;
	int		width;

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


	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETPOS,0,0);
	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETRANGE,0,MAKELPARAM(0, Steps));
	SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_SETSTEP,1,0);

	res=Method1(0,1,0,18,StartSplit,NumSplit,ConvertType);		// Step through all options

	Method1(0,1,0,18,res,1,ConvertType);					

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
		res=Method1(1,1,y,1,StartSplit,NumSplit,ConvertType);		// Step through all options
		Method1(1,1,y,1,res,1,ConvertType);
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


	for(y=0;y<144;y++)
	{
		for(x=0;x<160;x++)
		{
			raw[0][x][y][0] = *(pBitsdest+(143-y)*3*160+x*3+2);
			raw[0][x][y][1] = *(pBitsdest+(143-y)*3*160+x*3+1);
			raw[0][x][y][2] = *(pBitsdest+(143-y)*3*160+x*3);

			GBView=translate(raw[0][x][y]);

			raw[1][x][y][0] = GBView.rgbRed;
			raw[1][x][y][1] = GBView.rgbGreen;
			raw[1][x][y][2] = GBView.rgbBlue;
		}
	}
	


}



// Start X = 0 for Left / 1 for Right
// Width = 1 for half screen 2 = full screen
// StartY = 0 - 17 : Starting attribute block
// Height = Number of attribute blocks to check / process


int Method1(int StartX, int Width, int StartY, int Height, int StartJ, int FinishJ, int ConvertType)
{
	u32		Accum,width,x1,ts,tw,y2,x2,os;
	s32		x,y;
	s32		i,j;
	u8		col;


	BestQuantLine=0xffffffff;

	for(x=StartX;x<(StartX+Width);x++)
	{
		if (x==0)		//add Y-offset for left of image.
			os=1; 
		else
			os=0;

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
				SendDlgItemMessage(Ghdwnd,IDC_PROGRESS,PBM_STEPIT,0,0);

				for(x1=0;x1<4;x1++)
				{
					ts=TileOffset[x1];
					tw=TileWidth[x1];
			
					for(y2=0;y2<2;y2++)
					{
						for(x2=0;x2<tw;x2++)
						{
							for(i=0;i<3;i++)
							{
								*(Data+(tw*3*y2)+x2*3+i)=pic[x*80+ts+x2][y*2+y2-os][i];
							}
						}
					}
			
					switch(ConvertType)
					{
						case 0:
	
							to_indexed(Data,4,0,TileWidth[x1],2);			// Median Reduction No Dither
							break;
	
						case 1:
	
							to_indexed(Data,4,1,TileWidth[x1],2);			// Median Reduction With Dither
							break;
	
						case 2:
	
							wuReduce(Data,4,TileWidth[x1]*2);				// Wu Reduction
							break;
					}
			
					for(y2=0;y2<4;y2++)
					{
						IdealPal[x*4+x1][y][y2][0]=QuantizedPalette[y2][2];
						IdealPal[x*4+x1][y][y2][1]=QuantizedPalette[y2][1];
						IdealPal[x*4+x1][y][y2][2]=QuantizedPalette[y2][0];
					}
			
					for(y2=0;y2<2;y2++)
					{
						for(x2=0;x2<tw;x2++)
						{
							col=Picture256[y2*tw+x2];
			
							out[x*80+x2+ts][y*2+y2-os]=col;
			
							for(i=0;i<3;i++)
							{
								*(pBitsdest+(143-(y*2+y2-os))*3*160+(x*80+ts+x2)*3+i)=QuantizedPalette[col][i];
			
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




