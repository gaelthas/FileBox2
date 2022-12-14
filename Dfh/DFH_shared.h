////////////////////////////////////////////////////////////////////////
//
//                       FileBox eXtender v.2
// Copyright (C) 1999-2009  Greg Kochaniak, Hyperionics Technology LLC,
//                    http://www.hyperionics.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
////////////////////////////////////////////////////////////////////////

// DFH shared data

#define MAX_RECENT 32
#define MAX_BTNS 4 // have it always 2 more than we use!
#define MAX_HWDATA 128
#define MAX_ICON_SIZE  2048
#define MAX_EXCEPTIONS 64

#ifdef _WIN64
# define HPNAME _T("FbXHookProp64-389483jfsdkkjf259kei")
#else
# define HPNAME _T("FbXHookProp-389483jfsdkkjf259kei")
#endif

// dwFlags bits in HW_DATE
#define FBACT_POSTMSG	1		// Post a message in dwMsg
#define	FB_NO_PUSHPIN	2
#define FB_RESIZE_VONLY 4		// Resize this window vertically only
#define FB_RESIZE_HONLY 8		// Resize this window horizontally only
#define FB_NO_FBTNS		0x10	// No favorite or recent buttons
#define FB_NO_ROLLUP	0x20	// No roll up/roll down button
#define FB_WAS_TOPMOST	0x40	// Was topmost before roll-up
#define FB_RESIZED		0x80	// This window was already resized

// Buttons to display
#define BTN_PUSHPIN		1
#define BTN_FAV_FB		2
#define BTN_REC_FB		4
#define BTN_FAV_EX		8
#define BTN_REC_EX		0x10
#define BTN_ROLLUP		0x20
#define FLD_EXPLORE		0x8000

// WM_NULL message with the following wParam and lParam
// is sent to a rolled up window to unroll it:
#define WP_UNROLL_MSG 0xF0B17253
#define LP_UNROLL_MSG WP_UNROLL_MSG

// WM_NULL message with wParam and lParam as below
// means we're sending current directory to a file box
#define WP_CURDIR_MSG 0xF0B17255
#define LP_CURDIR_MSG WP_CURDIR_MSG


inline DWORD HexDigit(char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch + 10 - 'a';
	else if (ch >= 'A' && ch <= 'F')
		return ch + 10 - 'A';
	return 0;
}

#ifndef _WIN64
#  define DUMMY32(x) DWORD x;
#else
#  define DUMMY32(x)
#endif

#pragma pack(push)
#pragma pack(2)

typedef struct _hw_data
{
	BOOL	bHandleThisWindow;
	RECT	rctBtn[MAX_BTNS];
	RECT	rctWin;				// full window rectangle, in screen coords
	BYTE	nBtnType[MAX_BTNS];	// 0 - don't displ, 1 favorites, 2 recent, 3 pushpin, 4 rollup...
	BYTE	nBtnIcon[MAX_BTNS];	// icon number, starting from 0, 255 - no icon
	BYTE	nBtnState[MAX_BTNS];// For themed buttons: 0 normal, 1 hot etc., 0xFF undefined
	short	iPushed;			// index of caption btn. pushed, starting from 1, right.
	short	iClass;
	int		iButtonsRight;		// X coordinate of the first button's right edge
	DWORD	dwRelTime;			// button release time, needed for handling menu btns
	DWORD	dwProcId;			// Process id
	DWORD	dwFlags;
	DWORD	dwOldHeight;		// for roll-up/down
	WNDPROC OldHookedWindowProc; // for testing only here
		DUMMY32(dd5)

	union {
		HWND	hwndEdit;			// edit control to type the dir name into
		DWORD	dwMsg;				// message to send to a window
	};
		DUMMY32(dd6)
	HWND	hwndEdit2;			// For 0x47C / 0x480 uncertainty 
		DUMMY32(dd7)
	HWND	hwndBar;			// Toolbar in the title bar
		DUMMY32(dd8)
		DUMMY32(dd9)
	HWND	hwnd;				// KEEP Last!!! this hooked window handle, for double check
} HW_DATA;

#define FAVMNU_LEN	(2*256*256)

class DF_SHARED_DATA_COPY
{
public:
	DWORD	dwSize;			// size of this structure
	HWND	hDFhWnd;		// Main FbX window.
		DUMMY32(dd3)
	int		nPixelsLeft;	// Move buttons by extra pixels left
	int		nMaxPixelsLeft;	// Move extra if window maximized, for Office toolbar
	DWORD	dwButtons;		// flags, buttons to display: BTN_PUSHPIN, BTN_FAV_FB, BTN_REC_FB etc.
							// Explorer windows flag: FLD_EXPLORE - folders in explorer mode
	bool	bRecentPath;	// Display full path on Recent menu
	bool	bRolledTopmost;		// Do not use XP Style buttons
	bool	bRecAddNum;
	bool	bSwitchYZ; // add &1 etc. to menus
	bool	bSortRecent;	// Sort recent menu alphabetically
	bool	bSysTopmost;	// Add Topmost command to system menu
	bool	bSysRollup;		// Add Rollup command to system menu
	bool	bRecentFromWin; // Update recent folders from Windows "My Recent Documents"


	DWORD	dwDetailsView;	// Bit 1: Auto-switch to detailes view on std. file boxes, 
							// Bit 2: Auto-size columns
							// Bit 3: Sort descending if 1, ascending if 0
							// Bit 4: unused
							// Bit 5 - 8: Column number to sort by, mask: 0xF0 >> 4
	// Click through to get dir
	int		nWantClickDir;
	HWND	hwndWantClickDir;
		DUMMY32(dd4)
	
	WORD	wFavKey, wFavMod; // Favorites menu hot key and modifiers
	WORD	wRecKey, wRecMod; // Recent menu hot key and modifiers
	TCHAR	szAdd[16], szRemove[16]; // Add: and Remove: strings
	TCHAR	szOnTop[64], szRollUp[64];
	TCHAR	buf[2048];
	short	nRecent, nMaxRecent;
	short	nResizeFlags;	// 0 - no resize, 1 - resize
	short	nResizeWidth, nResizeHeight;
	int		nDeskRight, nDeskBottom, nMaxWidth, nMaxHeight;
	TCHAR	sRecent[MAX_RECENT][MAX_PATH];
	DWORD	dwFavDescOffset;	// offset to file strings in pcFavorites
	TCHAR	pcFavorites[FAVMNU_LEN]; // 128 kB should be enough...
	BYTE	bIconBits[2*MAX_BTNS][MAX_ICON_SIZE];	// Icon bits
	int		nExceptions;
	TCHAR	sExceptions[MAX_EXCEPTIONS][MAX_PATH + 4];
};

class DF_SHARED_DATA : public DF_SHARED_DATA_COPY
{
public:
	// The data below is specific to 32 or 64 bit sub-system
	int		iRecursion;
	DWORD	miscFlags1;
	bool	bWait, bWaitRes; // Instead of mutexes, which don't work for low priority processes like IE
	bool	bFlag1, bFlag2;	// unused yet...
	bool	m_bFindCtl;
	HW_DATA	hw[MAX_HWDATA]; // hooked window data
};

#pragma pack(pop)

#define SHMEMSIZE  (sizeof(DF_SHARED_DATA))
#define SHMEMSIZE_COPY (sizeof(DF_SHARED_DATA_COPY))

extern DF_SHARED_DATA *pShData;
extern DF_SHARED_DATA_COPY *pShDataCopy;

#pragma pack(push)
#pragma pack(2)

typedef struct {
    WORD  wVersion; 
    WORD  wOffset; 
    DWORD dwHelpId; 
} MENUEX_TEMPLATE_HEADER; 

typedef struct { 
    DWORD  dwType; 
    DWORD  dwState; 
    UINT   uId; 
    WORD   bResInfo; 
    WCHAR  szText[1]; 
    DWORD dwHelpId; 
} MENUEX_TEMPLATE_ITEM; 

#pragma pack(pop)


// Icon structures in ICO files, our source
#pragma pack( push )
#pragma pack( 1 )

typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
    ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

#pragma pack( pop )



// Icon structures in EXE and DLL resources (our target)
// #pragmas are used here to insure that the structure's
// packing in memory matches the packing of the EXE or DLL.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
   BYTE   bWidth;               // Width, in pixels, of the image
   BYTE   bHeight;              // Height, in pixels, of the image
   BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
   BYTE   bReserved;            // Reserved
   WORD   wPlanes;              // Color Planes
   WORD   wBitCount;            // Bits per pixel
   DWORD   dwBytesInRes;         // how many bytes in this resource?
   WORD   nID;                  // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

typedef struct 
{
   WORD            idReserved;   // Reserved (must be 0)
   WORD            idType;       // Resource type (1 for icons)
   WORD            idCount;      // How many images?
   GRPICONDIRENTRY   idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;

#pragma pack( pop )


