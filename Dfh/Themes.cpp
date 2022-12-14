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

#include "stdafx.h"
#include "DFH_shared.h"
#include "DFH.h"
#include "../resource.h"

inline int ClrDif(ULONG c1, ULONG c2)
{
	return (abs(GetRValue(c1) - GetRValue(c2)) + 
			abs(GetGValue(c1) - GetGValue(c2)) +
			abs(GetBValue(c1) - GetBValue(c2))
		);
}

BOOL IsThemed()
{
	if (g_VerInfo.dwMajorVersion < 5 || 
		g_VerInfo.dwMajorVersion == 5 && g_VerInfo.dwMinorVersion < 1)
		return FALSE;
	return IsAppThemed();
}

BOOL GetCaptionButtonBounds(HWND hwnd, RECT *pRct)
{
	if (g_haveDwmApi < 1)
		return FALSE;
	HRESULT hr = DwmGetWindowAttribute(
		hwnd,
		DWMWA_CAPTION_BUTTON_BOUNDS,
		(PVOID) pRct,
		sizeof(RECT)
	);
	//Msg(_T("\n\nCaptBounds for hwnd=%X: %d %d %d %d"), hwnd, pRct->left, pRct->right, pRct->top, pRct->bottom);
	return hr == S_OK;
}


BOOL DrawThemeButton(HWND hwnd, HDC hdc, RECT *pRect, BOOL bPushed) // int iStateId)
{
	HTHEME hTheme;
	HRESULT hr = S_FALSE;
	hTheme = OpenThemeData( hwnd, L"window");
	if (!hTheme)
		return FALSE;
	int iStateId = bPushed ? MINBS_PUSHED : (GetForegroundWindow() == hwnd ? MINBS_NORMAL : 5 /*MINBS_???*/);
	/*
	MINBS_???? = 5 - this is actually used for inactive toolbars!
	MINBS_DISABLED = 4
	MINBS_HOT = 2
	MINBS_NORMAL = 1
	MINBS_PUSHED = 3
	*/

	if (!bPushed) {
		// Is cursor inside?
		POINT ptcur;
		RECT  rctwin;
		GetCursorPos(&ptcur);
		FbxGetWindowRect(hwnd, &rctwin);
		ptcur.x -= rctwin.left;
		ptcur.y -= rctwin.top;
		if (PtInRect(pRect, ptcur)) {
			iStateId = MINBS_HOT;
		}
	}

    // Create a memory DC inside which we will scan the bitmap content 
    HDC hMemDC = CreateCompatibleDC(NULL); 
    if (hMemDC) 
    {
		int iWidth = pRect->right - pRect->left;
		int iHeight = pRect->bottom - pRect->top;
		// Create a 32 bits depth bitmap and select it into the memory DC 
		BITMAPINFOHEADER RGB32BITSBITMAPINFO = { 
			sizeof(BITMAPINFOHEADER), // biSize 
				iWidth,  // biWidth; 
				iHeight, // biHeight; 
				1,    // biPlanes; 
				32,    // biBitCount 
				BI_RGB,    // biCompression; 
				0,    // biSizeImage; 
				0,    // biXPelsPerMeter; 
				0,    // biYPelsPerMeter; 
				0,    // biClrUsed; 
				0    // biClrImportant; 
		}; 
		VOID * pbits32 = NULL; 
		HBITMAP hbm32 = CreateDIBSection(hMemDC, (BITMAPINFO *)	&RGB32BITSBITMAPINFO,
			DIB_RGB_COLORS, &pbits32, NULL, 0); 
		if (hbm32) 
		{ 
			HBITMAP holdBmp = (HBITMAP)SelectObject(hMemDC, hbm32);
			RECT rct;
			rct.left = rct.top = 0;
			rct.right = iWidth;
			rct.bottom = iHeight;
			BitBlt(hMemDC, 0, 0, iWidth, iHeight, hdc, pRect->left, pRect->top, SRCCOPY);
			hr = DrawThemeBackground (hTheme, hMemDC, WP_MINBUTTON, iStateId, &rct, 0);
			if (hr == S_OK) {
				// Get how many bytes per row we have for the bitmap bits (rounded up to 32 bits) 
				BITMAP bm32; 
				GetObject(hbm32, sizeof(bm32), &bm32); 
				while (bm32.bmWidthBytes % 4) 
					bm32.bmWidthBytes++; 
				// Scan each bitmap row from bottom to top (the bitmap is inverted vertically) 
				BYTE *p32 = (BYTE *)bm32.bmBits;
				int nMargin = 3;
				int nReplace = iWidth - 2*nMargin;
				for (int y = 2; y < iHeight/2+1; y++) 
				{
					ULONG *p = (ULONG *)p32 + y*iWidth + nMargin;
					ULONG c1 = *(p-1);
					ULONG c2 = *(p+nReplace);
					for (int i = 0; i < nReplace; i++)
						*(p+i) = RGB(GetRValue(c1) + i*(GetRValue(c2)-GetRValue(c1))/nReplace,
									 GetGValue(c1) + i*(GetGValue(c2)-GetGValue(c1))/nReplace,
									 GetBValue(c1) + i*(GetBValue(c2)-GetBValue(c1))/nReplace
									);
				}
				BitBlt(hdc, pRect->left, pRect->top, iWidth, iHeight, hMemDC, 0, 0, SRCCOPY);
			}
			SelectObject(hMemDC, holdBmp);
			DeleteObject(hbm32);
		}
		DeleteDC(hMemDC);
	}

	CloseThemeData(hTheme);
	return hr == S_OK;
}

BOOL DrawThemePushPin(HWND hwnd, HDC hdc, RECT *pRect, BOOL bPushed) // int iStateId)
{
	HTHEME hTheme;
	hTheme = OpenThemeData( hwnd, L"EXPLORERBAR");
	if (!hTheme)
		return FALSE;
	int iStateId = bPushed ? EBHP_SELECTEDNORMAL : EBHP_NORMAL;

	/*
	EBHP_HOT = 2
	EBHP_NORMAL = 1
	EBHP_PRESSED = 3
	EBHP_SELECTEDHOT = 5, 
	EBHP_SELECTEDNORMAL = 4
	EBHP_SELECTEDPRESSED = 6

	if (fPushed)
	iStateId = 4;
	else if (fDisabled)
	iStateId = 3;
	else if (fMouseOver)
	iStateId = 2;
	else if (fDefault)
	iStateId = 1;
	else
	iStateId = 0;
	*/
	
	HRESULT hr = DrawThemeBackground (hTheme, hdc, 
		EBP_HEADERPIN, iStateId, pRect, 0);

	CloseThemeData(hTheme);
	return hr == S_OK;
}

static bool IsCompositionTranslucent()
{
    DWORD color = 0;
    BOOL opaque = FALSE;

    return SUCCEEDED(::DwmGetColorizationColor(&color,
                                               &opaque)) && !opaque;
}

BOOL FbxGetWindowRect(HWND windowHandle, __out RECT *rct)
{
    if (!::IsWindow(windowHandle))
		return FALSE;

    const bool isDesktopCompositionEnabled = IsDesktopCompositionEnabled();

    RECT windowRect;

    if (!GetWindowRect(windowHandle, &windowRect))
    {
        return FALSE;
    }

    HRGN windowRegion = CreateRectRgn(0, 0, 0, 0);

    if (windowRegion == NULL)
    {
		rct->left = windowRect.left;
		rct->top = windowRect.top;
		rct->right = windowRect.right;
		rct->bottom = windowRect.bottom;
        return TRUE;
    }

    int regionResult = GetWindowRgn(windowHandle, windowRegion);

    if (COMPLEXREGION != regionResult && SIMPLEREGION != regionResult) // no region info
    {
        if (isDesktopCompositionEnabled)
        {
            ::DwmGetWindowAttribute(windowHandle,
                                    DWMWA_EXTENDED_FRAME_BOUNDS,
                                    &windowRect,
                                    sizeof (RECT));
        }
    }
    else
    {
        POINT point;
		point.x = windowRect.left;
		point.y = windowRect.top;

        GetRgnBox(windowRegion, &windowRect);
        OffsetRect(&windowRect, point.x, point.y);
    }
	DeleteObject(windowRegion);
    rct->left = windowRect.left;
    rct->top = windowRect.top;
	rct->right = windowRect.right;
	rct->bottom = windowRect.bottom;

	if (isDesktopCompositionEnabled && IsZoomed(windowHandle))
	{
		HMONITOR hMon = MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi;
		mi.cbSize = sizeof(mi);
		if (GetMonitorInfo(hMon, &mi))
			rct->top = mi.rcMonitor.top;
	}
    return TRUE;
}
