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

// DFH.h

extern ATOM  aDfPropName;
extern DF_SHARED_DATA *pShData;
extern OSVERSIONINFO g_VerInfo;
extern HINSTANCE hDllInst;

#ifdef _DEBUG
void Msg( LPCTSTR fmt, ... );
#else
inline void Msg( LPCTSTR fmt, ... ) {}
#endif // _DEBUG

// Themes.cpp functions
BOOL IsThemed();
BOOL GetCaptionButtonBounds(HWND hwnd, RECT *pRct);
BOOL FbxGetWindowRect(HWND windowHandle, __out RECT *rct);
BOOL DrawThemeButton(HWND hwnd, HDC hdc, RECT *pRect, BOOL bPushed);
BOOL DrawThemePushPin(HWND hwnd, HDC hdc, RECT *pRect, BOOL bPushed);

void RemoveLocalHooks();
void DrawButton(HDC hdc, HW_DATA *phd, int i, BOOL bPushed);
void GetCurrentWinDir(TCHAR *buf, HW_DATA *phd);
void GetDirName(TCHAR *buf2, TCHAR *name);
HMENU CreateRecentMenu();
void SwitchFbFolder(HW_DATA *phd, TCHAR *pcDir);

HDC GetBarWndDC(HW_DATA *phd);
bool IsDesktopCompositionEnabled();
ATOM RegisterBarWndClass();
void UnRegisterBarWndClass();
HWND CreateBarWindow(HW_DATA *phd);
void RollWindow(HW_DATA *phd);
void UnrollWindow(HW_DATA *phd);

void SetView(HWND hwndLV, DWORD dwView);
void ResizeFileBox(HW_DATA *phd);
void SortDetailedListColumn(HWND hwndParent, HWND hwndList);
void SwitchAndSortFileBox(HW_DATA *phd);
void SwitchFbFolder(HW_DATA *phd, TCHAR *pcDir);
void GetDirName(TCHAR *buf2, TCHAR *name);
HWND FindWindowWithClass(HWND wnd, const TCHAR* className, int id = 0);
void GetCurrentWinDir(TCHAR *buf, HW_DATA *phd);
void AutoFillDlg(HW_DATA *phd);

extern const UINT UM_FbxStartStop;
//extern HANDLE hResMutex;
extern BOOL bWin9x;
extern BOOL g_bThemed;
extern int g_haveDwmApi; // must be > 0 !

#define MAX_WNDCLASS  32
#define SYSCMD_ONTOP	( 0xD200 + 0x10 )
#define SYSCMD_ROLLUP   ( 0xD200 + 0x20 )
