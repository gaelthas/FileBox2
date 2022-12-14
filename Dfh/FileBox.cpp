////////////////////////////////////////////////////////////////////////
//
//                       FileBox eXtender v.2
// Copyright (C) 1999-2011  Greg Kochaniak, Hyperionics Technology LLC,
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
// Modified 2/23/2011 by Claudio Nicora
// http://coolsoft.altervista.org
// Fixed the following:
// - in Windows Explorer, selecting a preferred folder from FBX dropdown menu 
// causes another instance of Explorer to start (correctly showing the selected 
// folder)
// - "Click-switch file box folder" feature does not work
//
////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DFH_shared.h"
#include "DFH.h"
#include "../resource.h"

// SetView - sets a list view's window style to change the view. 
// hwndLV - handle to the list view control. 
// dwView - value specifying a view style. 
void SetView(HWND hwndLV, DWORD dwView) 
{ 
	// Get the current window style. 
	DWORD dwStyle = GetWindowLong(hwndLV, GWL_STYLE); 
	// Msg("dwStyle = 0x%x", dwStyle);

	// Only set the window style if the view bits have changed. 
	if ((dwStyle & LVS_TYPEMASK) != dwView) {
		SetWindowLong(hwndLV, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView); 
	}
} 

void ResizeFileBox(HW_DATA *phd)
{
	RECT r, rb, rc;
	HWND hwnd = phd->hwnd, hCtrl;
	static BOOL bHandleResizable = TRUE;
	int s, bResizable;

	Msg(_T("Trying to resize..."));
	if (!(pShData->nResizeFlags & 1) || (phd->dwFlags & FB_RESIZED)) {
		Msg(_T("Ret 1"));
		return;
	}
	bResizable = GetWindowLong(hwnd, GWL_STYLE) & WS_SIZEBOX;
	HWND hDlg = FindWindowEx(hwnd, NULL, _T("#32770"), NULL);

	if (!bResizable && (
		FindWindowEx(hwnd, NULL, _T("SysTabControl32"), NULL) || 
		hDlg && FindWindowEx(hDlg, NULL, _T("SysTabControl32"), NULL))
	   )
	   {
			Msg(_T("Ret 2"));
			return;
	}

	// Do not resize the new style boxes with "places" bar
	if (!bResizable && (hCtrl = GetDlgItem(hwnd, 0x4A0))) {
		TCHAR sClassName[256];
		*sClassName = 0;
		GetClassName(hCtrl, sClassName, 255);
		sClassName[255] = 0;
		if (strcmp(sClassName, _T("ToolbarWindow32")) == 0) {
			Msg(_T("Non-resizable, places bar."));
			return;
		}
	}

	phd->dwFlags |= FB_RESIZED;
	GetWindowRect(hwnd, &r);

	memcpy(&rb, &r, sizeof(RECT));
	Msg(_T("1: rb.top = %d, rb.bottom = %d"), rb.top, rb.bottom);
	if (!(bResizable && !bHandleResizable)) {
		if (!(phd->dwFlags & FB_RESIZE_VONLY))
			rb.right = rb.left + (pShData->nResizeWidth*(r.right - r.left))/100;
		if (!(phd->dwFlags & FB_RESIZE_HONLY) && 
			!FindWindowEx(hwnd, NULL, _T("USERITEM"), NULL)) // For QuickTime 4, paints text and lines on dlg
			rb.bottom = rb.top + (pShData->nResizeHeight*(r.bottom - r.top))/100;
	}
	GetClientRect(hwnd, &r);

	// Check if we fall out of the screen:
	if (rb.right - rb.left > pShData->nMaxWidth)
		rb.right = rb.left + pShData->nMaxWidth;
	if (rb.bottom - rb.top > pShData->nMaxHeight)
		rb.bottom = rb.top + pShData->nMaxHeight;
	s = pShData->nDeskBottom - rb.bottom;
	if (s < 0)
		rb.top += s, rb.bottom += s;
	s = pShData->nDeskRight - rb.right;
	if (s < 0)
		rb.left += s, rb.right += s;
			

	if (bResizable) {
		SetWindowPos(hwnd, HWND_TOP, rb.left, rb.top, 
			rb.right - rb.left, rb.bottom - rb.top,
			SWP_NOZORDER | SWP_DRAWFRAME | SWP_HIDEWINDOW);
		bHandleResizable = FALSE;
		Msg(_T("Ret 4"));
		return;
	}
	
	Msg(_T("Continue resizing..."));
	int iMoveRight = 100000;
	int iMoveBottom = 100000;
	int nID = 0;

	hCtrl = NULL;
	do {
		if (hCtrl = FindWindowEx(hwnd, hCtrl, _T("ToolbarWindow32"), NULL)) {
			nID = GetDlgCtrlID(hCtrl);
		}
	} while (hCtrl && !(nID == 0x440 || nID == 1));

	if (hCtrl != NULL) { 
		Msg(_T("Resize ver. 4"));
		// This is the new ver. 4 file box.
		/* Changes needed:
		0x471	- Combo box with "Look in" prompt - make wider only.
		class [ToolbarWindow32] - move to the right edge
		0x460	- list box, replaced with big box with files and folders, resize both X and Y.
		0x441	- "Files of type" prompt - move down
		0x470	- file types combo box - move down and make wider
		0x442	- "File name" prompt - move down
		0x480	- file edit box - move down and make wider
		0x1		- Open/Save button - move down and to the right
		0x2		- Cancel button - move down and to the right
		0x40E	- Help button - move down and to the right
		0x410	- read only check box
		*/
		int nRightEdge = 0, nBottomEdge = 0;

		// Resize the actual file box
		Msg(_T("2: rb.top = %d, rb.bottom = %d"), rb.top, rb.bottom);
		SetWindowPos(hwnd, HWND_TOP, rb.left, rb.top, 
			rb.right - rb.left, rb.bottom - rb.top,
			SWP_NOZORDER | SWP_DRAWFRAME | SWP_HIDEWINDOW);
		RECT rb_orig;
		GetWindowRect(hwnd, &rb_orig);
		GetClientRect(hwnd, &rb);

		// Move the Toolbar
		HWND hToolBar = hCtrl;
		if (hCtrl) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.left = rb.right - (r.right - rc.left);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// Resize the "Look in" combo box;
		if (hCtrl = GetDlgItem(hwnd, 0x471)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			iMoveRight = rc.right;
			rc.right = rb.right - (r.right - rc.right);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		/*
		// Move the "File name" prompt
		if (hCtrl = GetDlgItem(hwnd, 0x442)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		*/

		// move and resize file edit box
		if ((hCtrl = GetDlgItem(hwnd, 0x480)) || (hCtrl = GetDlgItem(hwnd, 0x47C))) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			nBottomEdge = rc.bottom;
			if (rc.right < iMoveRight)
				iMoveRight = rc.right;
			s = rc.bottom - rc.top;
			rc.top = rb.bottom - (r.bottom - rc.top);
			rc.bottom = rc.top + s;
			rc.right = rb.right - (r.right - rc.right);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		} 

		/*
		// 0x441	- "Files of type" prompt - move down
		if (hCtrl = GetDlgItem(hwnd, 0x441)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		*/

		// 0x470	- file types combo box - move down and make wider
		if (hCtrl = GetDlgItem(hwnd, 0x470)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			nBottomEdge = rc.bottom;
			s = rc.bottom - rc.top;
			rc.top = rb.bottom - (r.bottom - rc.top);
			rc.bottom = rc.top + s;
			rc.right = rb.right - (r.right - rc.right);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

		/*
		// 0x410	- read only check box
		if (hCtrl = GetDlgItem(hwnd, 0x410)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// 0x1		- Open/Save button - move down and to the right
		if (hCtrl = GetDlgItem(hwnd, 0x1)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.left = rb.right - (r.right - rc.left);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// 0x2		- Cancel button - move down and to the right
		if (hCtrl = GetDlgItem(hwnd, 0x2)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.left = rb.right - (r.right - rc.left);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// 0x40E	- Help button - move down and to the right
		if (hCtrl = GetDlgItem(hwnd, 0x40E)) {
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			rc.left = rb.right - (r.right - rc.left);
			rc.top = rb.bottom - (r.bottom - rc.top);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		*/

		// 0x460	- list box, replaced with big box with files and folders, resize both X and Y.
		if (hCtrl = GetDlgItem(hwnd, 0x460)) { 
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			iMoveBottom = rc.bottom;
			rc.right = rb.right - (r.right - rc.right);
			rc.bottom = rb.bottom - (r.bottom - rc.bottom);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

		// 0x461 shell view of files and folders
		if (hCtrl = GetDlgItem(hwnd, 0x461)) { 
			GetWindowRect(hCtrl, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			iMoveBottom = rc.bottom;
			nRightEdge = rc.right;
			rc.right = rb.right - (r.right - rc.right);
			rc.bottom = rb.bottom - (r.bottom - rc.bottom);
			SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
				rc.right - rc.left, rc.bottom - rc.top,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		}

		int nCtlFixed[] = {0x471, 0x47C, 0x480, 0x470, 0x460, 0x461, -1};
		int nID, i;
		hCtrl = GetTopWindow(hwnd);
		while (hCtrl) {
			if (hCtrl != hToolBar && hCtrl != hDlg) {
				nID = GetDlgCtrlID(hCtrl);
				for (i = 0; nCtlFixed[i] != -1; i++) {
					if (nID == nCtlFixed[i])
						break;
				}
				if (nID != nCtlFixed[i]) {
					GetWindowRect(hCtrl, &rc);
					MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
					if (rc.left >= iMoveRight) {
						rc.left = rb.right - (r.right - rc.left);
						SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
							rc.right - rc.left, rc.bottom - rc.top,
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					}
					if (rc.top >= iMoveBottom) {
						rc.top = rb.bottom - (r.bottom - rc.top);
						SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
							rc.right - rc.left, rc.bottom - rc.top,
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}
			}

			InvalidateRect(hCtrl, NULL, TRUE);
			hCtrl = GetNextWindow(hCtrl, GW_HWNDNEXT);
		}

		// Class #32770 - custom controls sub-dialog - move down and make wider
		if (hDlg) { 
			GetWindowRect(hDlg, &rc);
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
			int iTop = rc.top, iLeft = rc.left;

			if (hCtrl = GetTopWindow(hDlg)) {
				int nExtra = 0;
				if (phd->iClass == 7) {
					TCHAR buf[128];
					GetDlgItemText(hDlg, 0x809, buf, 127);
					PostMessage(hDlg, WM_COMMAND, (BN_CLICKED << 16) | 0x809, (LPARAM) NULL);
					if (strstr(buf, _T("<<"))) {
						PostMessage(hDlg, WM_COMMAND, (BN_CLICKED << 16) | 0x809, (LPARAM) NULL);
					}
					nExtra = 120;
				}

				SetWindowPos(hDlg, HWND_BOTTOM, rc.left, rc.top, 
					rb.right - rc.left, rb.bottom - rc.top + nExtra,
					SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS | 
					SWP_NOOWNERZORDER | SWP_NOSENDCHANGING | SWP_NOMOVE);
				
				while (hCtrl) {
					GetWindowRect(hCtrl, &rc);
					MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);

					if (nBottomEdge && rc.top > nBottomEdge)
						rc.top = rb.bottom - (r.bottom - rc.top) - iTop;
					else if (nRightEdge && rc.left >= nRightEdge ) 
						rc.left = rb.right - (r.right - rc.left) - iLeft;
					else
						rc.top = rb.bottom - (r.bottom - rc.top) - iTop;

					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					InvalidateRect(hCtrl, NULL, TRUE);
					hCtrl = GetNextWindow(hCtrl, GW_HWNDNEXT);
				}

				SetWindowPos(hwnd, HWND_TOP, rb_orig.left, rb_orig.top, 
					rb_orig.right - rb_orig.left, rb_orig.bottom - rb_orig.top,
					SWP_NOZORDER | SWP_DRAWFRAME | SWP_HIDEWINDOW);
			}
		}

		/*
		hCtrl = GetTopWindow(hwnd);
		while (hCtrl) {
			InvalidateRect(hCtrl, NULL, TRUE);
			hCtrl = GetNextWindow(hCtrl, GW_HWNDNEXT);
		}
		*/

	} else if ( GetDlgItem(hwnd, 0x460) /*File list*/ && 
				GetDlgItem(hwnd, 0x461) /*Folders list*/) 
	{ // Old ver. 3 file box
		// Resize the actual file box
		Msg(_T("Resize ver. 3"));
		hCtrl = GetDlgItem(hwnd, 0x480); // file name edit box
		GetWindowRect(hCtrl, &rc);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
		int s = rc.right;
		
		hCtrl = GetDlgItem(hwnd, 0x461); // foleders list-box
		GetWindowRect(hCtrl, &rc);
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
		iMoveRight = rc.right;
		iMoveBottom = rc.bottom;
		
		if (rc.left > s) { // otherwise file list is hidden and folders on left, forget it.
			int half_right, half_left;

			SetWindowPos(hwnd, HWND_TOP, rb.left, rb.top, 
				rb.right - rb.left, rb.bottom - rb.top,
				SWP_NOZORDER | SWP_DRAWFRAME | SWP_HIDEWINDOW);
			GetClientRect(hwnd, &rb);

			half_right = s + (rb.right - r.right)/2;
			s = rc.left - s;
			half_left = half_right + s;

			// Move and resize folders list-box
			if (rc.left > 0 && rc.top < r.bottom) {
				rc.left = half_left;
				rc.right = rb.right - (r.right - rc.right);
				rc.bottom = rb.bottom - (r.bottom - rc.bottom);
				SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
					rc.right - rc.left, rc.bottom - rc.top,
					SWP_NOZORDER | SWP_NOACTIVATE);
			}

			// Resize file edit box
			if (hCtrl = GetDlgItem(hwnd, 0x480)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (rc.left > 0) {
					rc.right = half_right;
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			// Move Folders: prompt
			if (hCtrl = GetDlgItem(hwnd, 0xffff)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (rc.left > 0) {
					rc.left = half_left;
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			// Move current folder text
			if (hCtrl = GetDlgItem(hwnd, 0x440)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (rc.left > 0)
					rc.left = half_left;
				if (rc.top >= r.bottom)
					rc.bottom = rb.bottom - (r.bottom - rc.bottom);
				SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
					rc.right - rc.left, rc.bottom - rc.top,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}

			// Resize files list box
			if (hCtrl = GetDlgItem(hwnd, 0x460)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (rc.left > 0) {
					rc.right = half_right;
					rc.bottom = rb.bottom - (r.bottom - rc.bottom);
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			// Move file types prompt
			if (hCtrl = GetDlgItem(hwnd, 0x441)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				rc.top = rb.bottom - (r.bottom - rc.top);
				SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
					rc.right - rc.left, rc.bottom - rc.top,
					SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			}

			// Move and resize file types combo box
			if (hCtrl = GetDlgItem(hwnd, 0x470)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				iMoveBottom = rc.top - 2;
				if (rc.left > 0) {
					s = rc.bottom - rc.top;
					rc.top = rb.bottom - (r.bottom - rc.top);
					rc.right = half_right;
					rc.bottom = rc.top + s;
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			// Move Drives prompt
			if (hCtrl = GetDlgItem(hwnd, 0x443)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (rc.left > 0 && rc.top < r.bottom) {
					rc.left = half_left;
					rc.top = rb.bottom - (r.bottom - rc.top);
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			// Move and resize drives combo box
			if (hCtrl = GetDlgItem(hwnd, 0x471)) {
				GetWindowRect(hCtrl, &rc);
				MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
				if (iMoveBottom < rc.bottom)
					iMoveBottom = rc.top - 2;
				if (rc.left > 0 && rc.top < r.bottom) {
					s = rc.bottom - rc.top;
					rc.left = half_left;
					rc.top = rb.bottom - (r.bottom - rc.top);
					rc.right = rb.right - (r.right - rc.right);
					rc.bottom = rc.top + s;
					SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
						rc.right - rc.left, rc.bottom - rc.top,
						SWP_NOZORDER | SWP_NOACTIVATE);
				}
			}

			int nCtlFixed[] = {0x461, 0x480, 0xffff, 0x440, 0x460, 0x441, 0x470, 0x443, 0x471, -1};
			int nID, i;
			hCtrl = GetTopWindow(hwnd);
			while (hCtrl) {
				nID = GetDlgCtrlID(hCtrl);
				for (i = 0; nCtlFixed[i] != -1; i++) {
					if (nID == nCtlFixed[i])
						break;
				}
				if (nID != nCtlFixed[i]) {
					GetWindowRect(hCtrl, &rc);
					MapWindowPoints(HWND_DESKTOP, hwnd, (POINT *) &rc, 2);
					if (rc.left >= iMoveRight) {
						rc.left = rb.right - (r.right - rc.left);
						SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
							rc.right - rc.left, rc.bottom - rc.top,
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					}
					if (rc.top >= iMoveBottom) {
						rc.top = rb.bottom - (r.bottom - rc.top);
						SetWindowPos(hCtrl, HWND_TOP, rc.left, rc.top, 
							rc.right - rc.left, rc.bottom - rc.top,
							SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}

				InvalidateRect(hCtrl, NULL, TRUE);
				hCtrl = GetNextWindow(hCtrl, GW_HWNDNEXT);
			}
		}
	} // Old ver. 3 file box
}

void SortDetailedListColumn(HWND hwndParent, HWND hwndList)
{
	int nCol = (pShData->dwDetailsView & 0xf0) >> 4;
	int nDescending = pShData->dwDetailsView & 0x4;
	if (nCol || nDescending) { // sort switch needed
		NMLISTVIEW nm;
		memset(&nm, 0, sizeof(nm));
		nm.hdr.code = LVN_COLUMNCLICK;
		nm.hdr.hwndFrom = hwndList;
		nm.hdr.idFrom = ::GetDlgCtrlID(hwndList);
		nm.iItem = -1;
		nm.iSubItem = 0;
		SendMessage(hwndParent, WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
		nm.iSubItem = nCol;
		SendMessage(hwndParent, WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
		if (/*nCol &&*/ nDescending) {
			SendMessage(hwndParent, WM_NOTIFY, nm.hdr.idFrom, (LPARAM) &nm);
		}
	}
}


void SwitchAndSortFileBox(HW_DATA *phd)
{
	if (phd->bHandleThisWindow && (phd->iClass == 1 || phd->iClass == 7) && pShData->dwDetailsView & 1) 
	{
		HWND hwnd = phd->hwnd;
		HWND hwndParent = GetDlgItem(hwnd, 0x461); // list view container?
		if (hwndParent) {
			HWND hwndList = FindWindowEx(hwndParent, NULL, _T("SysListView32"), NULL);
			if (hwndList) {
				SendMessage(hwnd, WM_COMMAND, 0xA004, 0); // switch to details view

				// Sort columns
				SortDetailedListColumn(hwndParent, hwndList);

				if (pShData->dwDetailsView & 2) {
					PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
					PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 1, LVSCW_AUTOSIZE);
					PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE);
					PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 3, LVSCW_AUTOSIZE);
				}
			}
		}
	}
}

void SwitchFbFolder(HW_DATA *phd, TCHAR *pcDir)
{
	HWND hwnd = phd->hwnd;
	HWND hwndEdit = phd->hwndEdit; // in case we delete phd...

	TCHAR buf[MAX_PATH + 20];
	//GetCurrentDirectory(MAX_PATH, buf);
	//if (!SetCurrentDirectory(pcDir))
	//	return;  // Don't do this, blocks file filters and document names!
	//SetCurrentDirectory(buf);

	if (phd->dwFlags & FBACT_POSTMSG) {
		// Currently this is only for "Browse for Folder" box
		// does not work with a \ at the end.
		Msg(_T("Sending 0x%X message, %s"), phd->dwMsg, pcDir);
		TCHAR *pc = pcDir + strlen(pcDir) - 1;
		if (*pc == '\\') *pc = 0;
		SendMessage(phd->hwnd, phd->dwMsg, TRUE, (LPARAM) pcDir);

	} else if (phd->iClass == 9 && hwndEdit) { // WinRAR 3
		::SetFocus(hwndEdit);
		SetWindowText(hwndEdit, pcDir);
        keybd_event(VK_RETURN, 0, 0, 0);
        keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0);

	} else if (phd->iClass == 10) { // WinZip 9 Extract to box
		SetWindowText(hwndEdit, pcDir);
		SendMessage(hwnd, WM_COMMAND, (BN_CLICKED << 16) | 0x1397, (LPARAM) GetDlgItem(hwnd, 0x1397));

	} else if (hwndEdit) {
		TCHAR buf2[5];
		WCHAR wString[MAX_PATH + 20];

		if (bWin9x && (phd->iClass == 2 || phd->iClass == 3 || phd->iClass == 8)) {
			memset(wString, 0, sizeof(wString));
#ifdef _UNICODE
			strcpy(wString, pcDir);
#else
			MultiByteToWideChar(
			  CP_ACP,         // code page
			  MB_PRECOMPOSED,         // character-type options
			  pcDir, // address of string to map
			  -1,       // number of bytes in string
			  wString,  // address of wide-character buffer
			  MAX_PATH + 19        // size of buffer - was 1024 here, bug!
			);
#endif
			pcDir = (TCHAR *) wString;
		}

		*buf = 0;
		BOOL bSetEdit2 = FALSE;
		do {
			// all of 3 lines below are to take away focus from 
			// selection list and set it to edit box instead.
			::SendMessage(hwndEdit, WM_LBUTTONDOWN, MK_LBUTTON, (4<<16) | 4);
			::SendMessage(hwndEdit, WM_LBUTTONUP, MK_LBUTTON, (4<<16) | 4);
			::SetFocus(hwndEdit);

			if (GetWindowText(hwndEdit, buf2, 5) > 1) {
				GetWindowText(hwndEdit, buf, MAX_PATH+20);
				bSetEdit2 = (hwndEdit == phd->hwndEdit2);
			}
			
			SetWindowText(hwndEdit, pcDir);
			if (phd->hwndEdit2 && hwndEdit != phd->hwndEdit2)
				hwndEdit = phd->hwndEdit2;
			else {
				break;
			}
		} while (1);
		hwndEdit = phd->hwndEdit;

		HWND btnHwnd = NULL;
		HWND hDlg = FindWindowEx(hwnd, NULL, _T("#32770"), NULL);

		if (phd->iClass == 4) {
			Msg(_T("Send message WM_CHAR"));
			SendMessage(hwndEdit, WM_CHAR, VK_RETURN, 0);

		} else if (phd->iClass == 7) { // CorelDraw or WordPerfect
			Msg(_T("CorelDraw or WordPerfect box..."));
			SendMessage(hwndEdit, WM_CHAR, VK_SPACE, 0);
			SendMessage(hwndEdit, WM_CHAR, VK_BACK, 0);
			SendMessage(hDlg, WM_COMMAND, (BN_CLICKED << 16) | IDOK, (LPARAM) btnHwnd);
		
		} else if (hDlg && (btnHwnd = GetDlgItem(hDlg, IDOK)) || ((btnHwnd = GetDlgItem(hwnd, IDOK))) && 
				   //IsWindowVisible(btnHwnd) && - invisible button and edit box still work
				   //IsWindowEnabled(btnHwnd) && - in WinRar "Add" file box.
				   phd->iClass != 6) 
		{
			Msg(_T("SendMessage(hwnd, WM_COMMAND, (BN_CLICKED << 16) | IDOK, (LPARAM) btnHwnd);"));
			SendMessage(hwnd, WM_COMMAND, (BN_CLICKED << 16) | IDOK, (LPARAM) btnHwnd);
		} else {
			Msg(_T("Send WM_KEYDOWN/WM_KEYUP messages"));
			SendMessage(hwndEdit, WM_KEYDOWN, VK_RETURN, 0);
			SendMessage(hwndEdit, WM_KEYUP, VK_RETURN, 0);
		}

		pcDir = buf;
		
		if (bWin9x && (phd->iClass == 2 || phd->iClass == 3 || phd->iClass == 8)) {
			unsigned char *pc;
			WCHAR *pw = wString;
			for (pc = (unsigned char *) pcDir; *pc; pc++)
				*pw++ = (*pc++ << 8);
			*pw = 0;
			memset(wString, 0, sizeof(wString));
#ifdef _UNICODE
			strcpy(wString, pcDir);
#else
			MultiByteToWideChar(
			  CP_ACP,         // code page
			  MB_PRECOMPOSED,         // character-type options
			  pcDir, // address of string to map
			  -1,       // number of bytes in string
			  wString,  // address of wide-character buffer
			  MAX_PATH + 19        // size of buffer - was 1024 here, bug!
			);
#endif
			pcDir = (TCHAR *) wString;
		}
		
		if (bSetEdit2) {
			SetWindowText(phd->hwndEdit2, pcDir);
			SetWindowText(hwndEdit, _T(""));
		} else {
			SetWindowText(hwndEdit, pcDir);
			if (phd->hwndEdit2)
				SetWindowText(phd->hwndEdit2, _T(""));
		}	
	} else {
		strcpy(buf, pcDir);
		BOOL bFolder = ::SetCurrentDirectory(buf); // in buf because SetCurrentDirectory() will add a '\\' ?
		TCHAR *pcPar = NULL;
		if (!bFolder && *pcDir == '\"') {
			pcPar = strchr(pcDir + 1, '\"');
			if (pcPar) {
				pcPar++;
				*pcPar = 0;
				pcPar++;
			}
		}

		BOOL bUseShellExecute = !(bFolder && (pShData->dwButtons & FLD_EXPLORE));

		if (!bUseShellExecute)
		{
			HWND h = FindWindowWithClass(hwnd, "Address Band Root", 0xA205);
			HWND hEdit = 0;
			if (h)
			{
				RECT r;
				GetWindowRect(h, &r);
				int d = (r.bottom - r.top)/2;
				::SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, (d<<16) | d);
				::SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, (d<<16) | d);
				hEdit = FindWindowWithClass(h, "Edit", 0xA205);
				if (!hEdit)
				{
					// Vista/7 Windows Explorer edit part of the address bar is created only after
					// clicking in an empty part of the breadcrumb control
					hEdit = FindWindowWithClass(h, "ToolbarWindow32", 0x03E9);
					if (hEdit) {
						// send a mouse message to the breadcrumb to make it create the edit control part
						SendMessage(hEdit, WM_MOUSEACTIVATE, 0, 0);
						SendMessage(hEdit, WM_LBUTTONDOWN, 0, 0);
						// now retry to find the edit control
						hEdit = FindWindowWithClass(h, "Edit", 0xA205);
					}
				}
				if (hEdit)
				{
					SetWindowText(hEdit, pcDir);
					SendMessage(hEdit, WM_KEYDOWN, VK_RETURN, 0);
					SendMessage(hEdit, WM_KEYUP, VK_RETURN, 0);
				}
			}
			if (!hEdit)
				bUseShellExecute = TRUE;
		}

		if (bUseShellExecute)
			ShellExecute(hwnd, _T("open"), pcDir, pcPar, _T(".\\"), SW_SHOW);
	}
	SwitchAndSortFileBox(phd);
}


typedef struct {
	TCHAR name[MAX_PATH];
	int nID;
} t_RecentMenuItem;

int _cdecl SortItems(const void *p1, const void *p2)
{
	t_RecentMenuItem *pi1 = (t_RecentMenuItem *) p1;
	t_RecentMenuItem *pi2 = (t_RecentMenuItem *) p2;
	return _stricmp(pi1->name, pi2->name);
}

HMENU CreateRecentMenu()
{
	HMENU hMenu = CreatePopupMenu();
	TCHAR buf2[MAX_PATH+20];
	int i;

	if (pShData->bRecentFromWin)
		SendMessage(pShData->hDFhWnd, WM_COMMAND, (WPARAM) 1241, (LPARAM) 0);

	if (pShData->bSortRecent)
	{
		t_RecentMenuItem items[MAX_RECENT];
		for (i = 0; i < pShData->nRecent; i++) {
			strcpy(buf2, pShData->sRecent[i]);
			GetDirName(buf2, items[i].name);
			items[i].nID = 100 + i;
		}
		qsort(&items, pShData->nRecent, sizeof(t_RecentMenuItem), SortItems);


		for (i = 0; i < pShData->nRecent; i++) {
			if (pShData->bRecAddNum) {
				TCHAR c = '1' + i;
				if (c > '9')
					c = 'A' - 1 + c - '9';
				wsprintf(buf2, _T("&%c %s"), c, items[i].name);
			} else
				strcpy(buf2, items[i].name);
			AppendMenu(hMenu, MF_STRING, items[i].nID, buf2);
		}
	} 
	else
	{
		TCHAR name[MAX_PATH+20];
		for (int i = 0; i < pShData->nRecent; i++) {
			strcpy(buf2, pShData->sRecent[i]);
			GetDirName(buf2, name);
			if (pShData->bRecAddNum) {
				char c = '1' + i;
				if (c > '9')
					c = 'A' - 1 + c - '9';
				wsprintf(buf2, _T("&%c %s"), c, name);
			} else
				strcpy(buf2, name);
			AppendMenu(hMenu, MF_STRING, 100+i, buf2);
		}
	}
	return hMenu;
}

void GetDirName(TCHAR *buf2, TCHAR *name)
{
	TCHAR *pc;
	pc = buf2 + strlen(buf2) - 1;
	if (*pc == '\\')
		*pc = 0;
	if (pShData->bRecentPath) {
		strcpy(name, buf2);
	} else {
		while (pc > buf2 && *pc != '\\')
			pc--;
		if (*pc == '\\') pc++;
		strcpy(name, pc);
		if (buf2[1] == ':')
			wsprintf(name + strlen(name), _T("  (%c:)"), islower(*buf2) ? _toupper(*buf2) : *buf2);
		else if (buf2[1] == '\\') {
			for (pc = buf2+2; pc < buf2+MAX_PATH && *pc != '\\'; pc++);
			if (pc < buf2+MAX_PATH) {
				for (pc++; pc < buf2+MAX_PATH && *pc != '\\'; pc++);
				if (pc < buf2+MAX_PATH) {
					*pc = 0;
					wsprintf(name + strlen(name), _T("  (%s)"), buf2);
				}
			}
		}
	}
}


HWND FindWindowWithClass(HWND wnd, const TCHAR* className, int id /* = 0 */)
{
	TCHAR wndClass[MAX_WNDCLASS + 1]; 

	::GetClassName(wnd, wndClass, MAX_WNDCLASS);

	if (strncmp(wndClass, className, MAX_WNDCLASS) == 0)
	{
		if (id == 0 || GetDlgCtrlID(wnd) == id)
			return wnd;
	}

	wnd = ::GetTopWindow(wnd);

	while (wnd != NULL)
	{
		HWND kid = FindWindowWithClass(wnd, className);

		if (kid != NULL) { return kid; } 
		
		wnd = ::GetNextWindow(wnd, GW_HWNDNEXT);
	}

	return NULL;
}



void GetCurrentWinDir(TCHAR *buf, HW_DATA *phd)
// buf must be at least MAX_PATH + 1 long!
{
	*buf = 0;
	if (phd != NULL) {
		HWND hwnd = phd->hwnd;
		if (phd->iClass == 5) // Explorer window
		{
			// Test for Vista folder:
			HWND h = hwnd;
			while (GetParent(h))
				h = GetParent(h);
			h = FindWindowEx(h, NULL, _T("WorkerW"), NULL);
			h = GetDlgItem(h, 0xA005);
			if (h)
				h = GetDlgItem(h, 0xA205);
			if (h)
				h = FindWindowEx(h, NULL, _T("msctls_progress32"), NULL);
			if (h)
				h = FindWindowEx(h, NULL, _T("Breadcrumb Parent"), NULL);

			if (h && (h = GetDlgItem(h, 0x3E9)))
			{
				GetWindowText(h, buf, MAX_PATH);
				TCHAR *pc = strstr(buf, ": ");
				if (pc)
					memmove(buf, pc+2, strlen(pc+2)+1);
				else
					*buf = 0;
			}
			else
			{
				// Command message 41477 toggles display of address bar,
				// thus making sure it exists

				for (int i = 0; i < 2; i++) {
					// Try twice, second time with address bar flicker
					hwnd = GetDlgItem(hwnd, 0xA005); // class WorkerW window
					if (hwnd) {
						hwnd = GetDlgItem(hwnd, 0xA005);	// class ReBarWindow32 window
						if (hwnd) {
							hwnd = GetDlgItem(hwnd, 0xA205);	// edit/combo box
							if (hwnd) {
								GetWindowText(hwnd, buf, MAX_PATH);
								break;
							}
						}
					}
					if (!hwnd) {
						// address bar flicker
						SendMessage(phd->hwnd, WM_COMMAND, 41477, 0);
						SendMessage(phd->hwnd, WM_COMMAND, 41477, 0);
						hwnd = phd->hwnd;
					}
				}

				if (buf[0] == 0) { // try to get from window's title
					GetWindowText(phd->hwnd, buf, MAX_PATH);
					buf[MAX_PATH] = 0;
					TCHAR *pc = strstr(buf, _T(":\\"));
					if (pc)
						pc--;
					else
						pc = strstr(buf, _T("\\\\"));
					if (pc)
						memmove(buf, pc, strlen(pc) + 1);
					else
						*buf = 0;
				}
			}

			if (_access(buf, 0) < 0)
			{
				*buf = 0;
			}

		} else if (phd->iClass == 9 && phd->hwndEdit) { // WinRAR v.3.x window
			GetWindowText(phd->hwndEdit, buf, MAX_PATH);

		} 
		/* 	There is some trouble with this with some applications
		else if (phd->iClass == 1 && !(phd->dwFlags & FBACT_POSTMSG)) { // try to send CDM_GETFOLDERPATH message
			int iRet = SendMessage(hwnd, (UINT) CDM_GETFOLDERPATH, MAX_PATH, (LPARAM) buf);
			Msg("CDM_GETFOLDERPATH, iRet = %d, buf = %s", iRet, buf);
		}
		*/

		/*
		if (*buf == 0) {
			// This gives me always C:\Documents and Settings\greg\Desktop\... path
			// Should I resolve it as a shortcut? No, does not help.
			HWND hwndList = FindWindowWithClass(hwnd, "SysListView32");
			if (hwndList)
			{
				LVITEM item = { 0 };
				item.mask = LVIF_PARAM;
				int count = ListView_GetItemCount(hwndList);
				if (count > 0)
				{
					item.iItem  = 0;
					item.lParam = 0;
					if (ListView_GetItem(hwndList, &item))
					{
						if (::SHGetPathFromIDList((LPCITEMIDLIST) item.lParam, buf))
						{
							//GetLinkTargetPath(buf, buf, MAX_PATH); // trying to get link target path does not work.
							Msg("From SHGetPathFromIDList: %s", buf);
						}
					}
				}
			}
		}
		*/
	} 
	
	if (*buf == 0)
		GetCurrentDirectory(MAX_PATH, buf);

	buf[MAX_PATH] = 0;
	if (*buf && buf[strlen(buf) - 1] != '\\')
		strcat(buf, _T("\\"));
	// Msg(_T("Current Dir found: %s"), buf); // Don't use Msg, as we're using its buf!!!
}

void AutoFillDlg(HW_DATA *phd)
{
#if 0 	// This is only code for Greg to auto-fill stupid IE password box
	HWND hwnd = phd->hwnd;
	char s[256];
	*s = 0;
	GetClassName(hwnd, s, 255);
	s[255] = 0;
	if (strcmp(s, "#32770") != 0)
		return;
	GetWindowText(hwnd, s, 255);
	s[255] = 0;
	if (strcmp(s, "Connect to hyperionics.com") != 0 &&
		strcmp(s, "Connect to www.hyperionics.com") != 0)
		return;
	HWND hSec = GetDlgItem(hwnd, 0x3EA);
	if (!hSec)
		return;
	HWND hChild = GetDlgItem(hSec, 0x3EB);
	Msg(_T("hChild = 0x%X"), hChild);
	if (!hChild)
		return;
	::SetWindowText(hChild, "hyperi");
	hChild = GetDlgItem(hSec, 0x3ED);
	Msg(_T("hChild2 = 0x%X"), hChild);
	if (!hChild)
		return;
	::SetWindowText(hChild, "Pkwm,aosj");
	PostMessage(hwnd, WM_COMMAND, (BN_CLICKED << 16) | IDOK, (LPARAM) GetDlgItem(hwnd, IDOK));

	//PostMessage(pShData->hDFhWnd, WM_COMMAND, ID_SENDKEYS, (LPARAM) hwnd);
#endif
}
