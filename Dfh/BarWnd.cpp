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

ATOM barClassId = 0;
#define BARWND_CLASS_NAME _T("FileBxBarWndClass_v2")

int g_haveDwmApi = -1;
LRESULT CALLBACK SubClassFunc(
			HWND hwnd,
            UINT Message,
            WPARAM wParam,
            LPARAM lParam);

bool IsDesktopCompositionEnabled()
{

	if (g_haveDwmApi == 0)
		return 0;

	if (g_haveDwmApi == -1)
	{
		HMODULE library = ::LoadLibrary(_T("dwmapi.dll"));
		if (library)
		{
			if (0 != ::GetProcAddress(library, "DwmIsCompositionEnabled"))
				g_haveDwmApi = 1;
			else
				g_haveDwmApi = 0;
			::FreeLibrary(library);
		}
		else
		{
			g_haveDwmApi = 0;
			return 0;
		}
	}

	BOOL enabled = FALSE;
	bool result = SUCCEEDED(DwmIsCompositionEnabled(&enabled)) && enabled;
    return result;
}

HDC GetBarWndDC(HW_DATA *phd)
{
	if (phd->hwndBar == (HANDLE) -1)
		return NULL;
	HDC hdc = GetWindowDC(phd->hwndBar);
	RECT rOwner, r;
	::FbxGetWindowRect(phd->hwnd, &rOwner);
	::GetWindowRect(phd->hwndBar, &r);
	SetViewportOrgEx(hdc, rOwner.left - r.left, rOwner.top - r.top, NULL);
	return hdc;
}

static LRESULT PaintBarWnd(HW_DATA *phd)
{
	HWND hOwner = phd->hwnd;
	if (!hOwner)
		return 1;

	PAINTSTRUCT pts;
	HDC hdc = BeginPaint(phd->hwndBar, &pts);
	if (hdc == NULL)
		return 1;
	RECT rOwner, r;
	::FbxGetWindowRect(hOwner, &rOwner);
	::GetWindowRect(phd->hwndBar, &r);
	SetViewportOrgEx(hdc, rOwner.left - r.left, rOwner.top - r.top, NULL);

	for (int i = 0; i < MAX_BTNS; i++) {
		if (phd->nBtnType[i]) {
			DrawButton(hdc, phd, i, FALSE);
		}
	}
	EndPaint(phd->hwndBar, &pts);
	return 0;
}

LRESULT CALLBACK BarWndProc(
	HWND hwndBar,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
	if (uMsg == WM_DESTROY)
		RemoveProp(hwndBar, (LPCTSTR) aDfPropName);

	if (uMsg == WM_DESTROY || uMsg == WM_NCDESTROY)
		return DefWindowProc(hwndBar, uMsg, wParam, lParam);

	LRESULT lRet;
	HWND hwnd = ::GetParent(hwndBar);
	if (!hwnd)
		return DefWindowProc(hwndBar, uMsg, wParam, lParam);
	int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
	HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;
	if (!phd)
		return DefWindowProc(hwndBar, uMsg, wParam, lParam);

	switch (uMsg) {	
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_SETCURSOR:
	case WM_ACTIVATE:
		return DefWindowProc(hwndBar, uMsg, wParam, lParam);
		//return MA_NOACTIVATEANDEAT;
	case WM_PAINT:
		return PaintBarWnd(phd);
	}

	RECT rctBtn, rctWin, rctBar;
	HDC hdc;
	int iPtInRect = 0, i;
	int xPos, yPos;

	if (phd->dwRelTime && GetTickCount() - phd->dwRelTime > 100) {
		if (phd->nBtnType[phd->iPushed - 1] <= 2) // only for menu buttons
			phd->iPushed = 0;
		phd->dwRelTime = 0;
	}

	memcpy(&rctWin, &phd->rctWin, sizeof(RECT)); // in case we delete phd...
	::GetWindowRect(hwndBar, &rctBar);
	xPos = GET_X_LPARAM(lParam) + rctBar.left - rctWin.left;  // horizontal position of cursor 
	yPos = GET_Y_LPARAM(lParam) + rctBar.top - rctWin.top;  // vertical position of cursor
	POINT pt; 
	pt.x = xPos; pt.y = yPos;	

	switch (uMsg) {
	case WM_LBUTTONDOWN:
		if (GetForegroundWindow() != hwnd)
			SetForegroundWindow(hwnd);
		for (i = 0; i < MAX_BTNS; i++) 
		{
			if (phd->nBtnType[i] && PtInRect(&phd->rctBtn[i], pt)) {
				memcpy(&rctBtn, &phd->rctBtn[i], sizeof(RECT)); // in case we delete phd...
				iPtInRect = i+1;
				break;
			}
		}

		if (!phd->iPushed && iPtInRect && (hdc = GetBarWndDC(phd))) 
		{
			int iRet;
			
			DrawButton(hdc, phd, iPtInRect-1, TRUE);
			phd->iPushed = iPtInRect;
			if (phd->nBtnType[iPtInRect - 1] <= 2) { // display menu
				for (i=0; pShData->bWaitRes && i < 10; i++)
					Sleep(100);
				if (pShData->bWaitRes) {
					ReleaseDC(hwndBar, hdc);
					Msg(_T("Wait failed in BarWndProc()"));
					return 0;
				}
				pShData->bWaitRes = true;
				pShData->iRecursion++;

				// Let's have the main DFolder window display the menu 
				// and execute commands... Or post commands to window's
				// queue.
				HMENU hMenu = NULL;
				HMENU hMainMnu = NULL;
				TCHAR buf2[MAX_PATH + 20], name[MAX_PATH], *pcDir;

				if (phd->nBtnType[iPtInRect - 1] == 1) {
					hMenu = LoadMenuIndirect((MENUTEMPLATE *) pShData->pcFavorites);
					GetCurrentWinDir(buf2, phd);
					if (*buf2) { 
						int i, n = GetMenuItemCount(hMenu);
						for (i = 0; i < n; i++) {
							iRet = GetMenuItemID(hMenu, i);
							if (iRet >= 10) {
								pcDir = pShData->pcFavorites + pShData->dwFavDescOffset + iRet;
								if (stricmp(pcDir, buf2) == 0)
									break;
							}
						}
						if (i < n) {
							GetMenuString(hMenu, i, name, MAX_PATH, MF_BYPOSITION);
							wsprintf(pShData->buf, _T("%s: \"%s\""), pShData->szRemove, name);
						} else {
							GetDirName(buf2, name);
							wsprintf(pShData->buf, _T("%s: \"%s\""), pShData->szAdd, name);
						}
						InsertMenu(hMenu, 1, MF_BYCOMMAND | MF_STRING, 9, pShData->buf);
					}
					hMainMnu = ::CreateMenu();
					::AppendMenu(hMainMnu, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, _T("R"));

				} else {
					hMenu = CreateRecentMenu();
				}

				iRet = 0;

				if (hMenu) {
					SetActiveWindow(hwnd);
					//BYTE rgbKeyState[256];
					//GetKeyboardState(rgbKeyState);
					//rgbKeyState[VK_LBUTTON] = 128;      // 0==UP, 128==DOWN
					//SetKeyboardState(rgbKeyState);
					
					iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, 
						rctBtn.left + rctWin.left, rctBtn.bottom + rctWin.top, 
						0, hwnd, NULL);
					DestroyMenu(hMainMnu ? hMainMnu : hMenu);
					DrawButton(hdc, phd, iPtInRect-1, FALSE);
					if (bWin9x) // must be WM_LBUTTONUP!!!
						PostMessage(hwndBar, WM_LBUTTONUP, HTCAPTION, lParam);
				}

				if (iRet >= 10 && iRet < FAVMNU_LEN) {
					if (phd->nBtnType[iPtInRect - 1] == 1)
						pcDir = pShData->pcFavorites + pShData->dwFavDescOffset + iRet;
					else
						pcDir = pShData->sRecent[iRet-100];
					SwitchFbFolder(phd, pcDir);
					
				} else if (iRet > 0 && iRet < 10) {
					typedef void (CALLBACK *ASFW) (DWORD);
					ASFW AllowSetForegroundWindow = NULL;

					if (iRet == 9) { // Add or Remove
						TCHAR *pc = pShData->buf + strlen(pShData->buf) + 1;
						// ^^ separated by \0
						GetCurrentWinDir(pc, phd);
					} else {
						HMODULE h = GetModuleHandle(_T("user32.dll"));
						if (h) {
							if (AllowSetForegroundWindow = (ASFW) GetProcAddress(h, "AllowSetForegroundWindow")) {
								DWORD dwProcId = 0;
								GetWindowThreadProcessId(pShData->hDFhWnd, &dwProcId);
								AllowSetForegroundWindow(dwProcId);
							}
						}
					}
					PostMessage(pShData->hDFhWnd, WM_COMMAND, ID_EDIT_FV, iRet);
					if (iRet != 9)
						SetForegroundWindow(pShData->hDFhWnd);
				}
				pShData->iRecursion--;
				//ReleaseMutex(hResMutex);
				pShData->bWaitRes = false;
			} else {
				// -- The below needed for regular command button
				SetCapture(hwndBar);
			}
			phd->dwRelTime = GetTickCount();
			ReleaseDC(hwndBar, hdc);
			return TRUE; // Do not process this message any more.
		}
		break;

	case WM_LBUTTONUP:
		if (phd->iPushed) 
		{
			for (int i = 0; i < MAX_BTNS; i++) {
				if (phd->nBtnType[i] && PtInRect(&phd->rctBtn[i], pt)) {
					iPtInRect = i+1;
					break;
				}
			}
			if (phd->nBtnType[phd->iPushed -1] > 2) { // this only for action button
				pShData->iRecursion++;
				ReleaseCapture();
				if (phd->iPushed == iPtInRect) {
					// Execute the action of this button
					if (phd->nBtnType[phd->iPushed -1] == 3) {
						BOOL bPushed = phd->nBtnIcon[phd->iPushed -1] == 2;
						phd->nBtnIcon[phd->iPushed -1] = bPushed ? 3 : 2;
						hdc = GetBarWndDC(phd);
						DrawButton(hdc, phd, phd->iPushed - 1, FALSE);
						ReleaseDC(hwndBar, hdc);
						if (bPushed) {
							SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						} else {
							SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						}

					} else if (phd->nBtnType[phd->iPushed -1] == 4) { // rollup
						BOOL bPushed = phd->dwOldHeight == 0;
						phd->nBtnIcon[phd->iPushed -1] = bPushed ? 5 : 4;
						hdc = GetBarWndDC(phd);
						DrawButton(hdc, phd, phd->iPushed - 1, FALSE);
						ReleaseDC(hwndBar, hdc);
						if (bPushed) {
							RollWindow(phd);
						} else {
							UnrollWindow(phd);
						}


					} else {
						hdc = GetBarWndDC(phd);
						DrawButton(hdc, phd, phd->iPushed - 1, FALSE);
						ReleaseDC(hwndBar, hdc);
						MessageBox(hwnd, _T("DFolder Action Button"), _T("DFolder"), MB_OK);
					}
				}
				phd->iPushed = 0;
				pShData->iRecursion--;

			}
		}
		break;

	case WM_MOUSELEAVE:
		if (g_bThemed) {
			hdc = GetBarWndDC(phd);
			for (int i = 0; i < MAX_BTNS; i++) {
				if (phd->nBtnType[i]) {
					int nState = PtInRect(&phd->rctBtn[i], pt) ? 1 : 0;
					if (nState != phd->nBtnState[i])
						DrawButton(hdc, phd, i, FALSE);
					phd->nBtnState[i] = nState;
				}
			}
			ReleaseDC(hwndBar, hdc);
		}
		break;

	case WM_MOUSEMOVE:
		hdc = GetBarWndDC(phd);
		if (g_bThemed) {
			for (int i = 0; i < MAX_BTNS; i++) {
				if (phd->nBtnType[i]) {
					int nState = PtInRect(&phd->rctBtn[i], pt) ? 1 : 0;
					if (nState != phd->nBtnState[i])
						DrawButton(hdc, phd, i, FALSE);
					phd->nBtnState[i] = nState;
				}
			}
			TRACKMOUSEEVENT trm;
			trm.cbSize = sizeof(trm);
			trm.dwFlags = TME_LEAVE | TME_CANCEL;
			trm.hwndTrack = hwndBar;
			trm.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&trm);
			trm.dwFlags = TME_LEAVE;
			TrackMouseEvent(&trm);
		}
		if (phd->iPushed && phd->nBtnType[phd->iPushed -1] > 2) // Don't do this for menu buttons
		{
			hdc = GetBarWndDC(phd);
			if (PtInRect(&phd->rctBtn[phd->iPushed-1], pt)) {
				DrawButton(hdc, phd, phd->iPushed - 1, TRUE);
			} else {
				DrawButton(hdc, phd, phd->iPushed - 1, FALSE);
			}
		}
		ReleaseDC(hwndBar, hdc);
		break;

	}

	lRet = DefWindowProc(hwndBar, uMsg, wParam, lParam);
	return lRet;
}


ATOM RegisterBarWndClass()
{
if (barClassId)
		return barClassId;
	WNDCLASS w;
	memset(&w, 0, sizeof(w));

	w.style = 0;
	w.lpfnWndProc = BarWndProc;
	w.hInstance = hDllInst;
	w.hCursor = LoadCursor(NULL, IDC_ARROW);
	w.hbrBackground = ::CreateSolidBrush(RGB(254,254,254));
	w.lpszClassName = BARWND_CLASS_NAME;

	barClassId = RegisterClass(&w);
	return barClassId;
}


void UnRegisterBarWndClass()
{
	if (barClassId)
		UnregisterClass(BARWND_CLASS_NAME, hDllInst);
	barClassId = 0;
}

HWND CreateBarWindow(HW_DATA *phd)
{
	phd->hwndBar = (HWND) -1;
	HWND hWndOwner = phd->hwnd;
	RECT r;
	GetWindowRect(hWndOwner, &r);
	phd->hwndBar = CreateWindowEx(
		//WS_EX_TRANSPARENT |
		WS_EX_NOACTIVATE | 
		WS_EX_NOPARENTNOTIFY | 
		WS_EX_TOOLWINDOW | 
		WS_EX_LAYERED,
		BARWND_CLASS_NAME,
		_T(""),
		WS_POPUP,
		r.right-200, r.top,
		100, 20,
		hWndOwner,
		NULL,
		NULL,
		NULL
		);
	if (phd->hwndBar)
	{
		Msg(_T("BarWnd %X, owner %X phd = %X, Proc.ID = %X"), phd->hwndBar, hWndOwner, phd, GetCurrentProcessId());
		SetLayeredWindowAttributes(phd->hwndBar, RGB(254,254,254), 255, LWA_COLORKEY | LWA_ALPHA);
	}
	return phd->hwndBar;
}

//
// Here is a sample subclass function.
//
LRESULT CALLBACK SubClassFunc(
			HWND hwnd,
            UINT Message,
            WPARAM wParam,
            LPARAM lParam)
{
	int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
	HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;
	if (!phd || !phd->OldHookedWindowProc)
		return 0;

	if ( Message == WM_WINDOWPOSCHANGING )
		return 0;

	LRESULT lRet = CallWindowProc(phd->OldHookedWindowProc, hwnd, Message, wParam, lParam);
	if ( Message == WM_NCHITTEST && (
		lRet == HTSIZE || lRet == HTBOTTOM || lRet == HTBOTTOMLEFT || lRet == HTBOTTOMRIGHT ||
		lRet == HTLEFT || lRet == HTRIGHT ||
		lRet == HTTOP || lRet == HTTOPLEFT || lRet == HTTOPRIGHT))
		lRet = HTCAPTION;

	return lRet;
}

void RollWindow(HW_DATA *phd)
{
	HWND hwnd = phd->hwnd;
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
		phd->dwFlags |= FB_WAS_TOPMOST;
	else
		phd->dwFlags &= ~FB_WAS_TOPMOST;
	FbxGetWindowRect(hwnd, &phd->rctWin);
	phd->dwOldHeight = phd->rctWin.bottom - phd->rctWin.top;
	int cy = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYSIZEFRAME)+4;
	phd->OldHookedWindowProc = (WNDPROC) SetWindowLongPtr(hwnd,
					 GWLP_WNDPROC, (LONG_PTR) SubClassFunc);
	SetWindowPos(hwnd, pShData->bRolledTopmost ? HWND_TOPMOST : NULL, 0, 0, 
		phd->rctWin.right - phd->rctWin.left, cy, 
		SWP_NOMOVE | (pShData->bRolledTopmost ? 0 : SWP_NOZORDER) | SWP_NOSENDCHANGING);
	InvalidateRect(phd->hwndBar, NULL, TRUE);
}

void UnrollWindow(HW_DATA *phd)
{
	HWND hwnd = phd->hwnd;
	if (phd->OldHookedWindowProc)
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) phd->OldHookedWindowProc);
	phd->OldHookedWindowProc = NULL;
	DWORD oh = phd->dwOldHeight;
	phd->dwOldHeight = 0;
	MoveWindow(hwnd, phd->rctWin.left, phd->rctWin.top, phd->rctWin.right - phd->rctWin.left, oh, TRUE);
	if (!(phd->dwFlags & FB_WAS_TOPMOST))
	{
		SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		InvalidateRect(phd->hwndBar, NULL, TRUE);
	}
}