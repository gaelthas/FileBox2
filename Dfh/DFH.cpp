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

// DFH.cpp : Defines the entry point for the DLL application.
// debug CPLs: Rundll32.exe shell32.dll,Control_RunDLL desk.cpl

#include "stdafx.h"

#include "DFH_shared.h"
#include "DFH.h"
#include "../resource.h"

HANDLE hMapObject = NULL;   /* handle to file mapping */ 
HINSTANCE hDllInst = NULL;
OSVERSIONINFO g_VerInfo;

DF_SHARED_DATA *pShData = NULL; /* address of shared memory */
#ifdef _WIN64
const UINT UM_FbxStartStop = ::RegisterWindowMessage(_T("FileBoxExtender64StartStopMsg"));
#else
const UINT UM_FbxStartStop = ::RegisterWindowMessage(_T("FileBoxExtenderStartStopMsg"));
#endif


// We need to have the hook handle hk available in all
// instances of the DLL, so we place it in a shared
// segment. After installing the hook, we will call
// SetHookValue() below to set this.
#pragma data_seg(".MYSEC")
//HHOOK hhkcw = (HHOOK) NULL; // for test only

HHOOK hhkcb = NULL;
HHOOK hhkMouse = NULL;
HHOOK hDebugHook = NULL;
BYTE fbxSharedData[SHMEMSIZE] = {0, 0, 0, 0}; // MUST BE INITIALIZED TO BE IN SHARED SEGMENT!
#pragma data_seg()
#pragma comment(linker, "/SECTION:.MYSEC,RWS")

ATOM  aDfPropName = 0;
BOOL bWin9x = FALSE;
BOOL g_bThemed = FALSE;
DF_SHARED_DATA_COPY *pShDataCopy;
//HANDLE hMutex = NULL;
//HANDLE hResMutex = NULL;
TCHAR szExeName[MAX_PATH + 20];


LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam0, LPARAM lParam0 );


#ifdef _DEBUG

void Msg( LPCTSTR fmt, ... )
{
    TCHAR    buff[1024];
    va_list  va;

    va_start(va, fmt);

    //
    // format message with header
    //

    strcpy( buff, _T("DFH: "));
    vsprintf( &buff[strlen(buff)], (const TCHAR *) fmt, va );
    strcat( buff, _T("\r\n"));

	if (pShData && pShData->hDFhWnd) {
		strncpy(pShData->buf, buff, 255);
		SendMessage(pShData->hDFhWnd, WM_COMMAND, (WPARAM) 0, (LPARAM) 0);
	}
	//FILE *fp = fopen(_T("C:\\Users\\greg\\AppData\\LocalLow\\FbxDbgLog.txt"), _T("a"));
	//if (fp)
	//{
	//	fprintf(fp, buff);
	//	fclose(fp);
	//}
	
	OutputDebugString( buff );
}

#endif

static int GetPhdIndex()
{
	if (!pShData)
		return 0;
	int i;

	for (i=0; pShData->bWait && i < 10; i++)
		Sleep(100);
	if (pShData->bWait) {
		Msg(_T("Wait failed in GetPhdIndex()"));
		return 0;
	}
	pShData->bWait = true;

	for (i = 0; i < MAX_HWDATA; i++) {
		if (pShData->hw[i].hwnd == NULL) {
			pShData->hw[i].hwnd = (HWND) 1;
			pShData->bWait = false;
			return i + 1;				// to avoid 0, which is "failed"
		}
	}

	pShData->bWait = false;
	return 0;
}

static void ClearWindow(HW_DATA *phd)
{
	if (phd->hwnd && IsWindow(phd->hwnd)) 
	{
		phd->bHandleThisWindow = FALSE;
		if (phd->dwOldHeight) {
			UnrollWindow(phd);
		}
		HMENU hmenu = GetSystemMenu(phd->hwnd, FALSE);
		if (hmenu) {
			DeleteMenu(hmenu, SYSCMD_ONTOP-1, MF_BYCOMMAND);
			DeleteMenu(hmenu, SYSCMD_ONTOP, MF_BYCOMMAND);
			DeleteMenu(hmenu, SYSCMD_ROLLUP, MF_BYCOMMAND);
		}
		if (phd->hwndBar && phd->hwndBar != (HWND) -1)
		{
			DestroyWindow(phd->hwndBar);
			Msg(_T("~BarWnd %X, h=%X, owner %X phd=%X, Proc.ID=%X"), phd->hwndBar, phd->hwnd, phd, GetCurrentProcessId());
		}
		RemoveProp(phd->hwnd, (LPCTSTR) aDfPropName);
	}
	memset(phd, 0, sizeof(HW_DATA));
}

void RemoveLocalHooks()
{
	if (!pShData)
		return;		
	int i;

	for (i=0; pShData->bWait && i < 10; i++)
		Sleep(100);
	if (pShData->bWait) {
		Msg(_T("Wait failed in RemoveLocalHooks()"));
		return;
	}
	pShData->bWait = true;
	
	DWORD dwProcId = GetCurrentProcessId();
	//Msg(_T("RemoveLocalHooks() for process %X"), dwProcId);

	for (i = 0; i < MAX_HWDATA; i++) {
		if (pShData->hw[i].hwnd) {
			HW_DATA *phd = &pShData->hw[i];
			if (phd->dwProcId == dwProcId) {
				ClearWindow(phd);
			}
		}
	}
	pShData->bWait = false;
	//ReleaseMutex(hMutex);
}


static void UpdateRecentMenu(TCHAR *pcDir)
{
	int i, nPos;
	TCHAR *pc = pcDir + strlen(pcDir) - 1;
	if (*pc != '\\')
		strcat(pcDir, _T("\\"));
	for (i = 0; i < pShData->nRecent; i++) {
		if (strcmp(pcDir, pShData->sRecent[i]) == 0)
			break;
	}
	if (!i && pShData->nRecent)
		return; // the same dir is already on top.
	
	nPos = i < pShData->nMaxRecent ? i : i-1;
	if (nPos < pShData->nRecent) {
		pShData->nRecent--;
	}
	for (i = nPos; i > 0; i--)
		strcpy(pShData->sRecent[i], pShData->sRecent[i-1]);

	strcpy(pShData->sRecent[0], pcDir);
	if (pShData->nRecent < pShData->nMaxRecent)
		pShData->nRecent++;
}

BOOL InitFBX(HINSTANCE hModule)
{
	pShData = (DF_SHARED_DATA*) &fbxSharedData;

	for (int i = 0; i < pShData->nExceptions; i++)
	{
		const char *pc = pShData->sExceptions[i];
		if (strstr(szExeName, strchr(pc, ' ') + 1))
		{
			int nEx = (HexDigit(pc[0])<<12) + (HexDigit(pc[1])<<8) + (HexDigit(pc[2])<<4) + HexDigit(pc[3]); //HexDigit(pc[0])*16 + HexDigit(pc[1]);
			if (nEx & 1) // not to handle this process
			{
				pShData = NULL;
				return FALSE;
			}
		}
	}

	g_VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&g_VerInfo);

	hDllInst = hModule;

	if (g_VerInfo.dwPlatformId != VER_PLATFORM_WIN32_NT) {
		bWin9x = TRUE;
	}
    
//#ifndef _WIN64
	/* Open a named file mapping object for 32-64 bit data exhange */ 
    hMapObject = OpenFileMapping( 
		FILE_MAP_ALL_ACCESS,      /* read/write access  */ 
		FALSE,
        _T("DFolderHook64-32"));      /* name of map object */
    if (hMapObject != NULL)
	{
		/* Get a pointer to the file-mapped shared memory. */ 
		pShDataCopy = (DF_SHARED_DATA *) MapViewOfFile( 
			hMapObject,     /* object to map view of    */ 
			FILE_MAP_WRITE, /* read/write access        */ 
			0,              /* high offset:   map from  */ 
			0,              /* low offset:    beginning */ 
			0);             /* default: map entire file */ 
	}
//#endif

    //DWORD  dwSiz;
	//if (!pShDataIn) {
	//	/* Create a named file mapping object */ 
	//	dwSiz = SHMEMSIZE;
	//	hMapObject = OpenFileMapping( 
	//		FILE_MAP_ALL_ACCESS,      /* read/write access  */ 
	//		FALSE,
	//		_T("DFolderHook"));      /* name of map object */ 
	//	if (hMapObject == NULL) 
	//		return FALSE; 

	//	// Get a pointer to the file-mapped shared memory.  
	//	pShData = (DF_SHARED_DATA *) MapViewOfFile( 
	//		hMapObject,     /* object to map view of    */ 
	//		FILE_MAP_WRITE, /* read/write access        */ 
	//		0,              /* high offset:   map from  */ 
	//		0,              /* low offset:    beginning */ 
	//		0);             /* default: map entire file */ 
	//	if (pShData == NULL) 
	//		return FALSE; 
	//}

	aDfPropName = GlobalAddAtom(HPNAME); // Increments usage count

	//hMutex = CreateMutex(NULL, FALSE, _T("M") HPNAME);
	//hResMutex = CreateMutex(NULL, FALSE, _T("R") HPNAME);
	g_bThemed = IsThemed();
	RegisterBarWndClass();
	return TRUE;
}

void CloseFBX()
{
	// RemoveInterceptAPIs();
	// Msg("Process Detach %u", GetCurrentProcessId()); crashes!
	RemoveLocalHooks();
    /* Unmap shared memory from the process's address space. */ 
    if (pShData) {
        // UnmapViewOfFile(pShData); 
		pShData = NULL;
	}
    /* Close the process's handle to the file-mapping object. */ 
	pShDataCopy = NULL;
    if (hMapObject)
        CloseHandle(hMapObject);
	GlobalDeleteAtom(aDfPropName); // Decrements usage count
	//CloseHandle(hMutex);
	//CloseHandle(hResMutex);
	UnRegisterBarWndClass();
}


BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE) hModule);
			GetModuleFileName(NULL, szExeName, MAX_PATH + 20);
			strlwr(szExeName);
			InitFBX(hModule);
			Msg(_T("Process Attach %u, %s %s"), GetCurrentProcessId(), szExeName, pShData ? "" : "(NOT HANDLED)");
            break;


        case DLL_PROCESS_DETACH:
			if (pShData)
				CloseFBX();
            break;
    }

    return TRUE;
}

void DrawButton(HDC hdc, HW_DATA *phd, int i, BOOL bPushed)
{
	if (phd->nBtnType[i] == 3 && phd->nBtnIcon[i] == 3)
		bPushed = TRUE;
	
	if (g_bThemed) {
		if (phd->nBtnType[i] == 3) { // PushPin
			DrawThemeButton(phd->hwnd, hdc, &phd->rctBtn[i], bPushed); 
			DrawThemePushPin(phd->hwnd, hdc, &phd->rctBtn[i], bPushed); 
			return;
		} else
			DrawThemeButton(phd->hwnd, hdc, &phd->rctBtn[i], bPushed); 
	} else
		DrawFrameControl(hdc, &phd->rctBtn[i], DFC_BUTTON, 
			DFCS_BUTTONPUSH | (bPushed ? DFCS_PUSHED : 0));

	int siz0 = phd->rctBtn[i].bottom - phd->rctBtn[i].top; // GetSystemMetrics(SM_CYSIZE) - 6;
	int siz;
	if (siz0 >= 20)
		siz = 18;
	else if (siz0 >= 17)
		siz = 16;
	else
		siz = 12;

	if (phd->nBtnIcon[i] < 255) {
		HICON hIcon;
		int n;
		GRPICONDIR *pgid = (GRPICONDIR *) pShData->bIconBits[phd->nBtnIcon[i]];

		if (!pgid->idCount) {
			hIcon = (HICON) LoadImage(hDllInst, MAKEINTRESOURCE(IDI_FAVORITES + phd->nBtnIcon[i]), 
				IMAGE_ICON, siz, siz, 0);
		} else {
			int nBytes = sizeof(GRPICONDIR) + (pgid->idCount - 1)*sizeof(GRPICONDIRENTRY); 
			int nSel = 0, nSizDif = 2048, j;
			for (n = 0; n < pgid->idCount; n++) {
				j = pgid->idEntries[n].bHeight - siz;
				if (j < 0) j = -j;
				if (j < nSizDif) {
					nSizDif = j;
					nSel = n;
					if (j == 0)
						break;
				}
			}
			for (n = 0; n <= nSel - 1; n++) 
				nBytes += pgid->idEntries[n].dwBytesInRes;

			hIcon = CreateIconFromResourceEx(pShData->bIconBits[phd->nBtnIcon[i]] + nBytes, 
				pgid->idEntries[nSel].dwBytesInRes,
				TRUE, 0x00030000, siz, siz, LR_DEFAULTCOLOR);
		}
		if (hIcon) {
			DrawIconEx(hdc, (phd->rctBtn[i].left + phd->rctBtn[i].right - siz)/2 + bPushed, 
				phd->rctBtn[i].top + (siz0 - siz)/2 + bPushed, hIcon,
				0, 0, 0, NULL, DI_NORMAL);
			DestroyIcon(hIcon);
		}
	}
}


void DrawButtons(HW_DATA *phd)
{
	HWND hwnd = phd->hwnd;
	BOOL bVis = IsWindowVisible(hwnd) && !::IsIconic(hwnd);

	if (phd->hwndBar == NULL && bVis)
		CreateBarWindow(phd);
	if (phd->hwndBar && phd->hwndBar != (HWND)-1)
	{
		RECT r, rParent;
		::FbxGetWindowRect(phd->hwndBar, &r);
		::FbxGetWindowRect(hwnd, &rParent);
		int iLastLeft = 0, iFirstRight = -99999;
		for (int i = 0; i < MAX_BTNS; i++) 
			if (phd->nBtnType[i])
			{
				iLastLeft = phd->rctBtn[i].left;
				if (iFirstRight == -99999)
					iFirstRight = phd->rctBtn[i].right;
			}
		int barWidth = iFirstRight - iLastLeft + 4;
		int barHeight = GetSystemMetrics(SM_CYSIZE);
		::MoveWindow(phd->hwndBar, 
			rParent.right - barWidth - phd->iButtonsRight,
			rParent.top + phd->rctBtn[0].top, 
			barWidth+2, barHeight, 
			TRUE);
		BOOL isBarVisible = IsWindowVisible(phd->hwndBar);
		if (isBarVisible && !bVis)
			::ShowWindow(phd->hwndBar, SW_HIDE);
		else if (!isBarVisible && bVis)
			::ShowWindow(phd->hwndBar, SW_SHOWNA);
	}
}


void InitButtons(HW_DATA *phd)
{
	HWND hwnd = phd->hwnd;

	FbxGetWindowRect(hwnd, &phd->rctWin);
	bool bCompEnabled  = IsDesktopCompositionEnabled();
	g_bThemed = IsThemed();
	if (g_bThemed)
	{
		RECT rbtn;
		if (GetCaptionButtonBounds(hwnd, &rbtn))
		{
			phd->iButtonsRight = phd->rctWin.right - phd->rctWin.left - rbtn.left + (bCompEnabled ? 2 : 0);
			phd->iButtonsRight += pShData->nPixelsLeft;
			WINDOWPLACEMENT wp;
			wp.length = sizeof(wp);
			if (GetWindowPlacement(phd->hwnd, &wp) && wp.showCmd == SW_MAXIMIZE) 
				phd->iButtonsRight += pShData->nMaxPixelsLeft;
			for (int i = 0; i < pShData->nExceptions; i++)
			{
				const char *pc = pShData->sExceptions[i];
				if (strstr(szExeName, strchr(pc, ' ') + 1) && pc[2] != ' ')
				{
					int n = HexDigit(pc[2])*16 + HexDigit(pc[3]);
					phd->iButtonsRight += n;
					break;
				}
			}
		}
	}

	if (phd->iClass == -2) // Office 2007
		phd->iButtonsRight = 104;

	if (!phd->iButtonsRight) {
		POINT p;
		LRESULT lRet; 
		p.x = phd->rctWin.right;
		p.y = phd->rctWin.top + 14;
		int nhits = 0, xhit;
		do {
			p.x -= 4;
			lRet = SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(p.x, p.y));
			if (lRet == HTCAPTION) {
				nhits++;
				if (nhits == 1)
					xhit = p.x;
			} else
				nhits = 0;
		} while (nhits < 3 && p.x >= phd->rctWin.left);
		if (nhits == 3) {
			p.x = xhit;
			do {
				p.x++;
				lRet = SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(p.x, p.y));
			} while (lRet == HTCAPTION && p.x < phd->rctWin.right);
			if (p.x >= phd->rctWin.right) {
				phd->iButtonsRight = 0;
			} else {
				phd->iButtonsRight = p.x - pShData->nPixelsLeft - phd->rctWin.left;

				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				if (GetWindowPlacement(phd->hwnd, &wp) && wp.showCmd == SW_MAXIMIZE) 
					phd->iButtonsRight -= pShData->nMaxPixelsLeft;
			}
		}
		if (phd->iButtonsRight)
		{
			phd->iButtonsRight = phd->rctWin.right - phd->rctWin.left - phd->iButtonsRight + (bCompEnabled ? 2 : 0);
		}
		for (int i = 0; i < pShData->nExceptions; i++)
		{
			const char *pc = pShData->sExceptions[i];
			if (strstr(szExeName, strchr(pc, ' ') + 1) && pc[2] != ' ')
			{
				int n = HexDigit(pc[2])*16 + HexDigit(pc[3]);
				phd->iButtonsRight += n;
				break;
			}
		}
	}

	LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
	int iBtnWidth = GetSystemMetrics(SM_CXSIZE) - 2;
	if (g_bThemed)
		iBtnWidth -= 2;
	if (!phd->iButtonsRight) {
		for (int i = 0; i < MAX_BTNS; i++) {
			phd->nBtnType[i] = 0;
			phd->nBtnState[i] = 0xFF;
		}
	}

	if ((pShData->dwButtons & BTN_PUSHPIN) && !(phd->dwFlags & FB_NO_PUSHPIN)) {
		phd->nBtnType[0] = 3;
		if (lExStyle & WS_EX_TOPMOST) {
			phd->nBtnIcon[0] = 3;
		} else
			phd->nBtnIcon[0] = 2;
	} else
		phd->nBtnType[0] = 0;

	if ((pShData->dwButtons & BTN_ROLLUP) && !(phd->dwFlags & FB_NO_ROLLUP)) {
		phd->nBtnType[1] = 4;
		phd->nBtnIcon[1] = phd->dwOldHeight ? 5 : 4;
	} else
		phd->nBtnType[1] = 0;

	if (!(phd->dwFlags & FB_NO_FBTNS)) {
		if (phd->iClass == 5 || phd->iClass == 9) { // Explorer or WinRAR
			if (pShData->dwButtons & BTN_FAV_EX)
				phd->nBtnType[2] = 1, phd->nBtnIcon[2] = 0;
			else
				phd->nBtnType[2] = 0;
			if (pShData->dwButtons & BTN_REC_EX)
				phd->nBtnType[3] = pShData->nMaxRecent ? 2 : 0, phd->nBtnIcon[3] = 1;
			else
				phd->nBtnType[3] = 0;

		} else if (phd->iClass > 0) {
			if (pShData->dwButtons & BTN_FAV_FB)
				phd->nBtnType[2] = 1, phd->nBtnIcon[2] = 0;
			else
				phd->nBtnType[2] = 0;
			if (pShData->dwButtons & BTN_REC_FB)
				phd->nBtnType[3] = pShData->nMaxRecent ? 2 : 0, phd->nBtnIcon[3] = 1;
			else
				phd->nBtnType[3] = 0;
		}
	} else
		phd->nBtnType[2] = phd->nBtnType[3] = 0;
	
	int j = 0;

	int iBtnRight = phd->rctWin.right - phd->rctWin.left - phd->iButtonsRight;
	
	
	int top = 0;
	if (!g_bThemed || !IsThemeActive())
		top = GetSystemMetrics((lStyle & WS_THICKFRAME) ? SM_CYSIZEFRAME : SM_CYDLGFRAME)+2;
	else if (!bCompEnabled)
	{
		BOOL bIsZoomed = IsZoomed(hwnd);
		if (lStyle & WS_THICKFRAME && !bIsZoomed)
		{
			MARGINS margs;
			HTHEME hTheme = g_VerInfo.dwMajorVersion > 5 ? OpenThemeData( hwnd, L"window") : NULL;
			if (hTheme)
			{
				HRESULT hr = GetThemeMargins(hTheme,
					NULL,
					WP_CAPTION,
					CS_ACTIVE,
					TMT_SIZINGMARGINS,
					NULL,
					(MARGINS *)&margs
				);
				if (hr == S_OK)
					top = margs.cyTopHeight + 1; //-3;
				else 
					top = GetSystemMetrics(SM_CYSIZEFRAME)+2;
				CloseThemeData(hTheme);
			}
			else
			{
				top = GetSystemMetrics(((lStyle & WS_THICKFRAME) && !bIsZoomed) ? SM_CYSIZEFRAME : SM_CYDLGFRAME);
				if (bIsZoomed)
					top--;
				else
					top += 2;
			}
		}
		else
		{
			top = GetSystemMetrics(SM_CYDLGFRAME) + (!(lStyle & WS_THICKFRAME) ? 2: 0);
			if (bIsZoomed)
				top--;
		}
	}
	
	int iBtnHeight = GetSystemMetrics(SM_CYSIZE) - (bCompEnabled ? 1 : 4);
	int iSpacing = (iBtnWidth + ((g_bThemed && !bCompEnabled)? 2 : 0));
	if (bCompEnabled)
		iSpacing--;
	for (int i = 0; i < MAX_BTNS; i++) {
		if (phd->nBtnType[i]) {
			phd->rctBtn[i].left = iBtnRight - (++j)*iSpacing;
			phd->rctBtn[i].top = top;
			phd->rctBtn[i].bottom = top + iBtnHeight;
			phd->rctBtn[i].right = phd->rctBtn[i].left + iBtnWidth;
		}
	}

	HMENU hmsys = GetSystemMenu(phd->hwnd, FALSE);
	if (hmsys) {
		DeleteMenu( hmsys, SYSCMD_ONTOP-1, MF_BYCOMMAND);
		if (pShData->bSysTopmost || pShData->bSysRollup)
			InsertMenu( hmsys, -1, MF_BYPOSITION | MF_SEPARATOR, SYSCMD_ONTOP-1, NULL );

		DeleteMenu( hmsys, SYSCMD_ONTOP, MF_BYCOMMAND);
		if (pShData->bSysTopmost)
		{
			InsertMenu( hmsys, -1, MF_BYPOSITION | MF_STRING | ((lExStyle & WS_EX_TOPMOST) ? MF_CHECKED : MF_UNCHECKED), SYSCMD_ONTOP, pShData->szOnTop );
			//CheckMenuItem(hmsys, SYSCMD_ONTOP, MF_BYCOMMAND | ((lExStyle & WS_EX_TOPMOST) ? MF_CHECKED : MF_UNCHECKED));
		}

		DeleteMenu( hmsys, SYSCMD_ROLLUP, MF_BYCOMMAND);
		if (pShData->bSysRollup)
		{
			InsertMenu( hmsys, -1, MF_BYPOSITION | MF_STRING | ((phd->dwOldHeight) ? MF_CHECKED : MF_UNCHECKED), SYSCMD_ROLLUP, pShData->szRollUp );
			//CheckMenuItem(hmsys, SYSCMD_ROLLUP, MF_BYCOMMAND | ((phd->dwOldHeight) ? MF_CHECKED : MF_UNCHECKED));
		}
	}
}


int TestClassName(HWND hwnd)
{
	TCHAR sClassName[1024];
	*sClassName = 0;
	GetClassName(hwnd, sClassName, 1000);
	sClassName[1000] = 0;
	// Msg(sClassName);
	// Except buggy programs...
	//Msg(_T("TestClassName(%s)"), sClassName);
	if (strstr(sClassName, _T("Photoshop")) == sClassName  || strcmp(sClassName, _T("PowerDVD Frame class")) == 0)
		return 0;

	int iClass = 0;

	if (strcmp(sClassName, _T("#32770")) == 0) { // Dialog box class
		iClass = 1;
		/* - not safe here
		HWND hwndParent = ::GetParent(hwnd);
		if (hwndParent) {
			::GetWindowText(hwndParent, sClassName, 255);
			sClassName[255] = 0;
			if (strstr(sClassName, "Corel PHOTO-PAINT") == sClassName)
				iClass = 6;
		}
		*/

	} else if ((strstr(sClassName, _T("bosa_sdm_Microsoft Word 8")) == sClassName ||
				strstr(sClassName, _T("bosa_sdm_Mso96")) == sClassName || // MS Office 97 (Access, PowerPoint)
				strstr(sClassName, _T("bosa_sdm_XL8")) == sClassName) && // Excel 97
				GetDlgItem(hwnd, 0x27) && 	// File name edit
				GetDlgItem(hwnd, 0x24) &&	// ??
				GetDlgItem(hwnd, 0x2C)
			)
	{
		iClass = 2;

	} else if (strstr(sClassName, _T("bosa_sdm_Mso96")) == sClassName &&
				GetDlgItem(hwnd, 0x30) && 	// 
				GetDlgItem(hwnd, 0x33) &&	// File name edit
				GetDlgItem(hwnd, 0x38)
		)
		// MS Visual Studio 2005
	{
		iClass = 2;

	} else if ((strstr(sClassName, _T("bosa_sdm_Microsoft Word 9")) == sClassName ||
				strstr(sClassName, _T("bosa_sdm_Mso96")) == sClassName ||
				strstr(sClassName, _T("bosa_sdm_XL9")) == sClassName) && // Excel 2000
				GetDlgItem(hwnd, 0x2D) &&
				GetDlgItem(hwnd, 0x30) && 	// File name edit
				GetDlgItem(hwnd, 0x35)
			) 
	{
		iClass = 3;

	} else if (strstr(sClassName, _T("TfFileDlg")) == sClassName)
	{
		// HomeSite 4 file dialogs
		// Msg("Found TfFileDlg window");
		iClass = 4;

	}
	
	else if (strstr(sClassName, _T("CabinetWClass")) == sClassName ||
			 strstr(sClassName, _T("ExploreWClass")) == sClassName || 
			 strstr(sClassName, _T("dopus.lister")) == sClassName ||  // Directory Opus 
			 strstr(sClassName, _T("EXPLORERPLUS"))) // PowerDesk
	{
		// Explorer folders, at least for IE 5
		// Msg("Found CabinetWClass (explorer) folder");
		iClass = 5;
	}

	else if (strstr(sClassName, _T("WinRarWindow")) == sClassName )	{
		// WinRAR folders
		Msg(_T("WinRAR folder"));
		iClass = 9;

	}
	
	else if (strstr(sClassName, _T("bosa_sdm_")) == sClassName 
				&&
				GetDlgItem(hwnd, 0x33) &&
				GetDlgItem(hwnd, 0x36) && 	// File name edit
				GetDlgItem(hwnd, 0x3B)
			) 
	{
		// MS Office XP
		Msg(_T("MS Office XP file box"));
		iClass = 8;
	}

	else if (strstr(sClassName, _T("XLMAIN")) == sClassName || 
		     strstr(sClassName, _T("OpusApp")) == sClassName ||
			 strstr(sClassName, _T("rctrl_renwnd32")) == sClassName)
	{
		if (FindWindowEx(hwnd, NULL, _T("MsoCommandBarDock"), NULL) || FindWindowEx(hwnd, NULL, _T("EXCEL2"), NULL)) {
			//Msg(_T("XLMAIN or OpusApp for Office 2007"));
			iClass = -2;
		}
	}

	else 
	{
		LONG l = GetWindowLong(hwnd, GWL_STYLE);
		LONG lex = GetWindowLong(hwnd, GWL_EXSTYLE);
		if ((l & WS_SYSMENU) && !(l & WS_CHILD) && 
			!(lex & WS_EX_TOOLWINDOW)) 
		{
			RECT rctWin;
			FbxGetWindowRect(hwnd, &rctWin);
			POINT p;
			LRESULT lRet;
			p.x = (rctWin.left + rctWin.right)/2;
			p.y = rctWin.top + 14;
			// Photoshop CS 2 Error!!!
			lRet = SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(p.x, p.y));
			if (lRet == HTCAPTION || !IsWindowVisible(hwnd)) 
			{
				iClass = -1; // for other top level window we want pushpin button
			}
		}
	}

	return iClass;
}


void PostProcessMessage(HW_DATA *phd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = phd->hwnd;

	if (phd && phd->dwOldHeight && 
		msg == WM_NULL && wParam == WP_UNROLL_MSG && lParam == LP_UNROLL_MSG) 
	{
		UnrollWindow(phd);
		return;
	}

	if (phd && msg == WM_NULL && wParam == WP_CURDIR_MSG && lParam == LP_CURDIR_MSG)
	{
		SwitchFbFolder(phd, pShData->buf);
	}

	if (hhkcb && phd && !phd->bHandleThisWindow && phd->iClass != 9999 &&
		(msg == UM_FbxStartStop && wParam == 1 ||
		 msg == WM_SETTEXT ||
		 msg == WM_NCPAINT || 
		 msg == WM_INITDIALOG || 
		 msg == WM_NCACTIVATE)) 
	{
		BOOL bHook = FALSE;
		{
			for (int i = 0; i < pShData->nExceptions; i++)
			{
				const char *pc = pShData->sExceptions[i];
				if (strstr(szExeName, strchr(pc, ' ') + 1))
				{
					int nEx = HexDigit(pc[0])*16 + HexDigit(pc[1]);
					if (nEx & 1)
						phd->iClass = 9999; // not to handle this window
					else 
					{
						if (!(nEx & (FB_RESIZE_VONLY | FB_RESIZE_HONLY))) // do not resize
							phd->dwFlags |= FB_RESIZED;
						else if ((nEx & (FB_RESIZE_VONLY | FB_RESIZE_HONLY)) == FB_RESIZE_VONLY)
							phd->dwFlags |= FB_RESIZE_VONLY;
						else if ((nEx & (FB_RESIZE_VONLY | FB_RESIZE_HONLY)) == FB_RESIZE_HONLY)
							phd->dwFlags |= FB_RESIZE_HONLY;
						if (nEx & FB_NO_PUSHPIN)
							phd->dwFlags |= FB_NO_PUSHPIN;
						if (nEx & FB_NO_ROLLUP)
							phd->dwFlags |= FB_NO_ROLLUP;
						if (nEx & FB_NO_FBTNS)
							phd->dwFlags |= FB_NO_FBTNS;
					}
				}
			}
		}

		switch (phd->iClass) {
		case 1: // #32770, dialog box
			{
				// Test for stupid CorelDraw/WordPerfect file boxes...
				HWND hDlg = FindWindowEx(hwnd, NULL, _T("#32770"), NULL);
				HWND h = NULL, h1 = NULL, h2 = NULL, h3 = NULL, h4 = NULL;

				if (hDlg && strstr(szExeName, _T("\\winrar.exe")) == NULL && strstr(szExeName, _T("maker.exe")) == NULL) {
					h = GetDlgItem(hDlg, 0x81C); // Combo box with  0x3E9 edit ctrl in CorelDraw
					if (h == NULL) {
						do {
							h = FindWindowEx(hDlg, h, _T("ComboBox"), NULL); // first combo-box
						} while (h && !GetDlgItem(h, 0x3E9));
					}
				}
				if (hDlg && h &&
					GetDlgItem(h, 0x3E9) &&		// Combo box with  0x3E9 edit ctrl
					FindWindowEx(hDlg, NULL, _T("AfxOleControl42"), NULL))
				{
					phd->iClass = 7; // CorelDraw file box
					phd->hwndEdit = h; // Combo box with  0x3E9 edit ctrl
					//phd->hwndEdit = GetDlgItem(phd->hwndEdit, 0x3E9);
					bHook = TRUE;

				} 
				else if ((h = FindWindowEx(hwnd, NULL, _T("DUIViewWndClassName"), NULL)))
				{
					if (
						GetDlgItem(hwnd, 0x442) && // File Name static
						(h = GetDlgItem(hwnd, 0x47C)) && // File name combo box
						GetDlgItem(hwnd, 0x441) && // File of type static
						GetDlgItem(hwnd, 0x1) && // Open/Save button
						GetDlgItem(hwnd, 0x2) && // Cancel button
						FindWindowEx(hwnd, NULL, _T("WorkerW"), NULL)) // Navigation bar
					{
						// Vista Open dlg
						phd->hwndEdit = h; // GetDlgItem(hwnd, 0x47C);
						bHook = TRUE;
					}
					else if (
						(h1 = FindWindowEx(h,  NULL, _T("DirectUIHWND"), NULL)) &&
						(h2 = FindWindowEx(h1, NULL, _T("FloatNotifySink"), NULL)) &&
						(h3 = FindWindowEx(h2, NULL, _T("ComboBox"), NULL)) &&
						(h4 = GetDlgItem(h3, 0x3E9))) // edit box
					{
						// Vista Save As dlg
						phd->hwndEdit = h4;
						bHook = TRUE;
					}
				}
				else if (GetDlgItem(hwnd, 0x139C) && // Extract to: static text
					GetDlgItem(hwnd, 0x1395) && // Extract to: combo box
					GetDlgItem(hwnd, 0x1397) && // browse to button
					GetDlgItem(hwnd, 0x1005)) // create new folder button
				{
					// This is WinZip 9 Extract box...
					phd->iClass = 10; // CorelDraw file box
					phd->hwndEdit = GetDlgItem(hwnd, 0x1395); // Combo box with  0x3E9 edit ctrl
					bHook = TRUE;

				} 
				else if (GetDlgItem(hwnd, 0x442) &&		// "File name" prompt
					GetDlgItem(hwnd, 0x470) &&		// Doc. types combo box
					GetDlgItem(hwnd, 0x441) &&		// "Files of type" prompt
					GetDlgItem(hwnd, 0x471) &&		// Look in/Save in box
					GetDlgItem(hwnd, 0x443)) 		// "Look in" prompt
				{
					if (h = GetDlgItem(hwnd, 0x47C))
					{
						// There is no other way to find out if the most recent file list is used with
						// 0x47C combo box, or straight edit box 0x480... Both are present and both seem
						// to be at this time invisible. We must use both.
						// this is Win2000 new (ver. 5) file box
						Msg(_T("Edit box is 0x47C"));
						phd->hwndEdit = h; // GetDlgItem(hwnd, 0x47C);
						phd->hwndEdit2 = GetDlgItem(hwnd, 0x480); 
						bHook = TRUE;
					}
					else if (h = GetDlgItem(hwnd, 0x480))	// File name edit 
					{
						// ver. 4 or lower file box
						Msg(_T("Edit box is 0x480"));
						phd->hwndEdit = h;
						bHook = TRUE;
					}

				} 
				else if (// GetDlgItem(hwnd, 0x3744) && // edit box, optional
						 GetDlgItem(hwnd, 0x3741) && // folder selection listbox "SysTreeView32"
						 (GetDlgItem(hwnd, 0x3743) || GetDlgItem(hwnd, 0x3746)) && // 0x3746 is "New Folder" btn on Win2000
						 (FindWindowEx(hwnd, NULL, _T("SHBrowseForFolder ShellNameSpace Control"), NULL) || 
						  FindWindowEx(hwnd, NULL, _T("SysTreeView32"), NULL) && GetDlgItem(hwnd, 0x3743)))
				{
					// "Browse for Folder"
					phd->dwFlags = FBACT_POSTMSG;
					phd->dwMsg = BFFM_SETSELECTIONA;
					bHook = TRUE;
				} 
				else if (
						GetDlgItem(hwnd, 0x32D0) && // "File name" prompt
						GetDlgItem(hwnd, 0x32CE) && // file name combo-box
						GetDlgItem(hwnd, 0x32D1) && // "Files of type" prompt
						GetDlgItem(hwnd, 0x32CA) && // file type combo box
						GetDlgItem(hwnd, 0x32CF) && // "Look in" prompt
						GetDlgItem(hwnd, 0x32C9))	// look in combo box
				{
					// AutoCAD 2000 file box
					phd->hwndEdit = GetDlgItem(hwnd, 0x32CE);
					bHook = TRUE;
				}
				//Msg(_T("#32770, iClass = %d, bHook = %d"), phd->iClass, bHook);
			}
			break;

		case 2: // MS Office 97 or VS 2005
			if (GetDlgItem(hwnd, 0x27) && 	// File name edit
				GetDlgItem(hwnd, 0x24) &&	// ??
				GetDlgItem(hwnd, 0x2C)
			)
			{
				phd->hwndEdit = GetDlgItem(hwnd, 0x27);
				bHook = TRUE;
			}
			else if (GetDlgItem(hwnd, 0x30) && 	// 
					 GetDlgItem(hwnd, 0x33) &&	// File name edit
					 GetDlgItem(hwnd, 0x38))
			{ // MS Visual Studio 2005
				phd->hwndEdit = GetDlgItem(hwnd, 0x33);
				bHook = TRUE;
			}

			break;

		case 3: // MS Office 2000
			if (GetDlgItem(hwnd, 0x2D) &&
				GetDlgItem(hwnd, 0x30) && 	// File name edit
				GetDlgItem(hwnd, 0x35)
			   ) 
			{
				phd->hwndEdit = GetDlgItem(hwnd, 0x30);
				bHook = TRUE;
			}
			break;

		case 4: // HomeSite 4
			{
				Msg(_T("Hooking TfFileDlg window"));
				// Search for a control with "TdxSavedCombo" class name
				HWND hwnd2 = GetTopWindow(hwnd);
				while (hwnd2) {
					TCHAR sClassName[256];
					*sClassName = 0;
					GetClassName(hwnd2, sClassName, 255);
					sClassName[255] = 0;
					if (strstr(sClassName, _T("TdxSavedCombo")) == sClassName) {
						phd->hwndEdit = GetDlgItem(hwnd2, 0x3E9);
						bHook = TRUE;
						break;
					}
					hwnd2 = GetNextWindow(hwnd2, GW_HWNDNEXT);
				}
			}
			break;

		case 5: // Explorer folders
			// Search for sub-window with class name "SHELLDLL_DefView"
			{
				HWND hwnd2 = GetTopWindow(hwnd);
				TCHAR buf[256];
				while (hwnd2) {
					*buf = 0;
					GetClassName(hwnd2, buf, 256);
					buf[255] = 0;
					if (strcmp(buf, _T("SHELLDLL_DefView")) == 0) {
						bHook = TRUE;
						break;
					}
					hwnd2 = GetNextWindow(hwnd2, GW_HWNDNEXT);
				}
			}
			break;

		/* - this is taken out for now. Not safe to call GetWindowText() in 
			 TestClassName()
		case 6: // Corel PHOTO-PAINT 
			bHook = TRUE;
			phd->hwndEdit = GetDlgItem(hwnd, 0x480);
			break;
		*/

		case 8: // MS Office XP
			if (GetDlgItem(hwnd, 0x33) &&
				GetDlgItem(hwnd, 0x36) && 	// File name edit
				GetDlgItem(hwnd, 0x3B)
			   ) 
			{
				phd->hwndEdit = GetDlgItem(hwnd, 0x36);
				bHook = TRUE;
			}
			break;

		case 9: // WinRAR 3
			{
				HWND hwnd = phd->hwnd;
				hwnd = GetDlgItem(hwnd, 0xF); // ReBarWindow32
				if (hwnd)
					phd->hwndEdit = GetDlgItem(hwnd, 0x10); // ComboBoxEx32				
			}
			break;

		case -1: // other windows with caption, where we want pushpin btn
		case -2:
			bHook = TRUE;
			break;
		}

		if (phd->iClass < 0 && 
			((phd->dwFlags & (FB_NO_PUSHPIN | FB_NO_ROLLUP)) == (FB_NO_PUSHPIN | FB_NO_ROLLUP))) {
			phd->iClass = 0;
			bHook = FALSE;

		} else if (phd->iClass > 0 && !bHook && (msg == WM_NCPAINT || msg == UM_FbxStartStop && wParam == 1))
		{
			LONG l = GetWindowLong(hwnd, GWL_STYLE);
			LONG lex = GetWindowLong(hwnd, GWL_EXSTYLE);
			if ((l & WS_SYSMENU) && (l & WS_CAPTION)==WS_CAPTION && 
				!(l & WS_CHILD) && !(lex & WS_EX_TOOLWINDOW)) 
			{
				if (phd->iClass != 5 && phd->iClass != 9) // will this fix the problem of folder btns on explorer?
					phd->iClass = -1; // for other top level window we want pushpin button
				bHook = TRUE;
			}
		}
		
		if (bHook) {
			// Msg(" - Mark this window for handling");
			phd->bHandleThisWindow = TRUE;
			
			if (phd->dwOldHeight == 0)
				phd->iButtonsRight = 0;
			InitButtons(phd);
			DrawButtons(phd);
		}
	}

	if (phd->iClass == 9999)
		return;

	switch (msg) {
	case WM_SIZE:
		if (phd->bHandleThisWindow && phd->dwOldHeight == 0 && phd->hwndBar)
		{
			phd->iButtonsRight = 0;
			InvalidateRect(phd->hwndBar, NULL, TRUE);
		}
		break;

	case WM_THEMECHANGED:
		if (phd->bHandleThisWindow)
		{
			if (phd->hwndBar)
				::ShowWindow(phd->hwndBar, SW_HIDE);
			phd->iButtonsRight = 0;
			FbxGetWindowRect(phd->hwnd, &phd->rctWin);
			InitButtons(phd);
			DrawButtons(phd);
		}
		break;

	case WM_NCACTIVATE: //0x86
		if (phd->bHandleThisWindow && phd->hwndBar)
			InvalidateRect(phd->hwndBar, NULL, FALSE);

	case WM_WINDOWPOSCHANGED: // 0x47 WM_WINDOWPOSCHANGED works better on WinXP XP style
	case WM_SETTEXT:
	case WM_SETICON:
	//case WM_NCPAINT:
	//case WM_ACTIVATE: //0x6
		if (phd->bHandleThisWindow) {
			FbxGetWindowRect(phd->hwnd, &phd->rctWin);
			InitButtons(phd);
			DrawButtons(phd);
			if (/*msg == WM_ACTIVATE && */pShData->nWantClickDir > 0 && phd->iClass > 0 && !(phd->iClass == 5 || phd->iClass == 9)) {
				pShData->hwndWantClickDir = phd->hwnd;
				if (pShDataCopy)
					pShDataCopy->hwndWantClickDir = phd->hwnd;
			}
			AutoFillDlg(phd);
		}
		break;

	case WM_INITDIALOG:
		if (phd->bHandleThisWindow && (phd->iClass == 1 || phd->iClass == 7)) {
			SwitchAndSortFileBox(phd);
			if (!(phd->dwFlags & FBACT_POSTMSG)) // to avoid resizing "Browse for folder"
				ResizeFileBox(phd);
		}
		break;

	/*
	case WM_COMMAND:
		if (phd && phd->bHandleThisWindow && (phd->iClass == 1 || phd->iClass == 7) &&
			(pShData->dwDetailsView & 1)) 
		{
			HWND hwndParent = GetDlgItem(hwnd, 0x461); // list view container?
			if (hwndParent && wParam != 0xA002) {
				HWND hwndList = FindWindowEx(hwndParent, NULL, "SysListView32", NULL);
				if (hwndList) {
					//Msg("WM_COMMAND wParam = 0x%x", wParam);
					// SortDetailedListColumn(hwndParent, hwndList);
					if ((pShData->dwDetailsView & 2) && ListView_GetItemCount(hwndList)) {
						PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
						PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 1, LVSCW_AUTOSIZE);
						PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE);
						PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 3, LVSCW_AUTOSIZE);
					}
				}
			}
		}
		break;
	*/

	
	/*  
	// WM_NOTIFY calls do not work, maybe they are only sent to a sub-window
	// with custom controls
	case WM_NOTIFY:
		if (phd && phd->bHandleThisWindow && (phd->iClass == 1 || phd->iClass == 7) &&
			(pShData->dwDetailsView & 1)) 
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;
			if (pnmh) {
				
				if (pnmh->code == CDN_TYPECHANGE || pnmh->code == CDN_FOLDERCHANGE) 
				{
					Msg("Got WM_NOTIFY code = %d from id = %d", pnmh->code, pnmh->idFrom);

					HWND hwndList = GetDlgItem(hwnd, 0x461); // list view container?
					hwndList = FindWindowEx(hwndList, NULL, "SysListView32", NULL);
					if (hwndList)
					{
						if (hwndList) {
							PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 0, LVSCW_AUTOSIZE);
							PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 1, LVSCW_AUTOSIZE);
							PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE);
							PostMessage(hwndList, LVM_SETCOLUMNWIDTH, 3, LVSCW_AUTOSIZE);
						}
					}
				}				
			}
		}
		break;
	*/

	}

}


LRESULT CALLBACK DebugProc(
  int nCode,      // hook code
  WPARAM wParam,  // hook type
  LPARAM lParam   // debugging information
)
{
	// The sole purpose of this hook is to prevent passing of
	// LB_GETSELITEMS list box message to WH_CALLWNDPROCRET hook
	// procedure, which is broken under NT4, really not my fault!
	if (nCode >= 0 && pShData) {
		try {
			DEBUGHOOKINFO *pdi = (DEBUGHOOKINFO *) lParam;

			if (pdi) {
				switch (wParam) {
				case WH_CALLWNDPROCRET:
					{
						CWPRETSTRUCT *pcw = (CWPRETSTRUCT *) pdi->lParam;
						if (pcw) {
							switch (pcw->message) {
							case WM_NCACTIVATE:
							case WM_NCPAINT:
							case WM_INITDIALOG:
							case WM_DESTROY:
							case WM_MOVE:
							case WM_SIZE:
							case WM_WINDOWPOSCHANGED:
							// case WM_COMMAND:
							// case WM_NOTIFY:	// safer not to send it there...
							case WM_SETTEXT:
								break;

							default:
								return 1;
							}
						}
					}

				default:
					break;
				}
			}
		} catch (...) {
			Msg(_T("Exception in DebugProc()"));
			return 1;
		}
	}

	LRESULT lRes = CallNextHookEx(
		hDebugHook,	// handle to current hook
		nCode,	// hook code passed to hook procedure
		wParam,	// value passed to hook procedure
		lParam 	// value passed to hook procedure
	   );

	return lRes;

}


LRESULT CALLBACK CallWndRetProc(
  int nCode,     // hook code
  WPARAM wPar, // current-process flag
  LPARAM lPar  // address of structure with message data
)
{
	if (pShData && nCode >= 0 && hhkcb) {
		try {
			CWPRETSTRUCT *pcw = (CWPRETSTRUCT *) lPar;
			if (pcw && pcw->hwnd && IsWindow(pcw->hwnd)) 
			{
				HWND hwnd = pcw->hwnd;
				DWORD dwProcId;

				GetWindowThreadProcessId(hwnd, &dwProcId);

				if (dwProcId == GetCurrentProcessId()) 
				{
					int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
					HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;

					if (!nphd &&
						(pcw->message == UM_FbxStartStop && wPar == 1 ||
						 pcw->message == WM_NCPAINT || 
						 pcw->message == WM_INITDIALOG || 
						 pcw->message == WM_NCACTIVATE ))
					{
						int iClass = TestClassName(hwnd);
						// Add our property to all activated windows, this will
						// prevent unnecessary multiple tests

						if (iClass && (nphd = GetPhdIndex())) {
							phd = &pShData->hw[nphd - 1];
							memset(phd, 0, sizeof(HW_DATA) - sizeof(HWND));
							phd->dwProcId = GetCurrentProcessId();
							phd->hwnd = hwnd;
							phd->iClass = iClass;
						} else
							nphd = -1;
						if (!SetProp(hwnd, (LPCTSTR) aDfPropName, (HANDLE) nphd))
							Msg(_T("SetProp() failed for nphd = %d"), nphd);
					}

					if (nphd) {
						if (pcw->message == WM_DESTROY) 
						{
							if (phd) {
								if (phd->hwnd == pShData->hwndWantClickDir)
								{
									pShData->hwndWantClickDir = NULL;
									if (pShDataCopy)
										pShDataCopy->hwndWantClickDir = NULL;
								}
								if (phd->bHandleThisWindow && pShData->nMaxRecent) {
									TCHAR buf[MAX_PATH + 1];
									//GetCurrentDirectory`(MAX_PATH, buf);
									GetCurrentWinDir(buf, phd);
									buf[MAX_PATH] = 0;
									UpdateRecentMenu(buf);
								}
								if (phd->hwndBar && phd->hwndBar != (HWND)-1)
									DestroyWindow(phd->hwndBar);
								memset(phd, 0, sizeof(HW_DATA));
							}
							RemoveProp(hwnd, (LPCTSTR) aDfPropName);
						} 
						else if (phd && pcw->message == UM_FbxStartStop && pcw->wParam == 0)
						{
							ClearWindow(phd);
							return 0;
						}
						else if (phd && phd->iClass) 
						{
							PostProcessMessage(phd, pcw->message, pcw->wParam, pcw->lParam);
						}
					}
				}
			}
		} catch (...) {
			Msg(_T("Exception in CallWndRetProc()"));
		}
	}

  LRESULT lRes = CallNextHookEx(
		hhkcb,  // handle to current hook
		nCode,	// hook code passed to hook procedure
		wPar,	// value passed to hook procedure
		lPar 	// value passed to hook procedure
	   );

	return lRes;
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam0, LPARAM lParam0 )
// This is now really WH_GETMESSAGE GetMsgProc() hook procedure, for stupid
// "user controls" in Visual Basic, which block WH_MOUSE hook.
{


	MSG *pMsg = (MSG *) lParam0; /* for WH_GETMESSAGE */
	
	if (pShData && nCode == HC_ACTION && wParam0 == PM_REMOVE && pMsg && pMsg->hwnd)
	{
		UINT message = pMsg->message; /* for WH_GETMESSAGE */
		int nHotKey = 0;

		if (message == WM_SYSCOMMAND) {
			HWND hwnd = pMsg->hwnd;
			int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
			if (nHotKey && nphd <= 0) {
				while (hwnd = ::GetParent(hwnd)) {
					if ((nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName)) > 0)
						break;
				}
			}
			HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;

			if (phd && phd->bHandleThisWindow)
			{
				LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
				int nID = (pMsg->wParam & 0xfff0);
				if (nID == SYSCMD_ONTOP)
				{
					if (lExStyle & WS_EX_TOPMOST)
					{
						SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					}
					else
					{
						SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					}
				}
				else if (nID == SYSCMD_ROLLUP)
				{
					if (phd->dwOldHeight)
					{
						UnrollWindow(phd);
					}
					else
					{
						RollWindow(phd);
					}
				}
			}
		} else if (message == WM_KEYDOWN) {
			if (pMsg->wParam == pShData->wFavKey || pMsg->wParam == pShData->wRecKey) {
				HWND hwnd = pMsg->hwnd;
				int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
				if (nphd <= 0) {
					while (hwnd = ::GetParent(hwnd)) {
						if ((nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName)) > 0)
							break;
					}
				}
				HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;
				if (phd && phd->bHandleThisWindow) {
					WORD mod = 0;
					if (GetAsyncKeyState(VK_SHIFT) >> ((sizeof(SHORT) * 8) - 1))
						mod |= HOTKEYF_SHIFT;
					if (GetAsyncKeyState(VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1))
						mod |= HOTKEYF_CONTROL;
					if (GetAsyncKeyState(VK_MENU) >> ((sizeof(SHORT) * 8) - 1))
						mod |= HOTKEYF_ALT;

					POINT pp;
					int nSearch;
					RECT rpp;
					FbxGetWindowRect(phd->hwndBar, &rpp);
					rpp.left -= phd->rctWin.left;
					rpp.top -= phd->rctWin.top;
					if (pMsg->wParam == pShData->wFavKey && mod == pShData->wFavMod && (nSearch=1) ||	// favorites
						pMsg->wParam == pShData->wRecKey && mod == pShData->wRecMod && (nSearch=2))		// recent
					{
						for (int i = 0; i < MAX_BTNS; i++) {
							if (phd->nBtnType[i] == nSearch) { // favorites
								pp.x = (phd->rctBtn[i].left + phd->rctBtn[i].right)/2 - rpp.left;
								pp.y = (phd->rctBtn[i].top + phd->rctBtn[i].bottom)/2 - rpp.top;
								SetFocus(phd->hwndBar);
								PostMessage(phd->hwndBar, WM_LBUTTONDOWN, 0, (pp.y << 16)|pp.x);
								nHotKey = i+1;
								break;
							}
						}
					}
				}
			} else if (pShData->bSwitchYZ) {
				// This works to replace keys in Polish keyboard layout, if I need it
				if (pMsg->wParam == 'Y' && (pMsg->lParam & 0xff0000) == 0x2C0000) {
					pMsg->wParam = 'Z';
					//Msg("Y, scan code = %X", pMsg->lParam & 0xff0000);
				} else if (pMsg->wParam == 'Z' && (pMsg->lParam & 0xff0000) == 0x150000) {
					pMsg->wParam = 'Y';
					//Msg("Z, scan code = %X", pMsg->lParam & 0xff0000);
				}
			}
		} 
		
		if (message == WM_LBUTTONDOWN || message == WM_NCLBUTTONDOWN ||
		    message == WM_MOUSEMOVE || message == WM_LBUTTONUP || 
			message == WM_NCMOUSEMOVE)
		{
			MOUSEHOOKSTRUCT msh; /* for WH_GETMESSAGE */
			LPMOUSEHOOKSTRUCT pM = &msh; /* for WH_GETMESSAGE */ // = (MOUSEHOOKSTRUCT *) lParam;

			/* for WH_GETMESSAGE */
			pM->hwnd = pMsg->hwnd;
			HWND hwnd = pM->hwnd;
			pM->pt = pMsg->pt;
			pM->dwExtraInfo = 0;
			pM->wHitTestCode = HTCAPTION; // SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pM->pt.x), pM->pt.y));
			/* for WH_GETMESSAGE */

			int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
			if (nHotKey && nphd <= 0) {
				while (hwnd = ::GetParent(hwnd)) {
					if ((nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName)) > 0)
						break;
				}
			}
			HW_DATA *phd = nphd > 0 ? &pShData->hw[nphd - 1] : NULL;

			if (pShData && pShData->nWantClickDir && pM->hwnd && 
				(message == WM_LBUTTONDOWN || message == WM_NCLBUTTONDOWN))
			{
				if (pShData->hwndWantClickDir || pShDataCopy && pShDataCopy->hwndWantClickDir)
				{
					if (pShData->hwndWantClickDir == NULL)
						pShData->hwndWantClickDir = pShDataCopy->hwndWantClickDir;
					BOOL bCont = TRUE;
					hwnd = pM->hwnd;
					DWORD dwProcIdCurr = 0, dwProcIdWantCD = 0;
					GetWindowThreadProcessId(hwnd, &dwProcIdCurr);
					GetWindowThreadProcessId(pShData->hwndWantClickDir, &dwProcIdWantCD);
					bCont = (BOOL) (dwProcIdCurr != dwProcIdWantCD);
					
					do {
						if (hwnd == pShData->hwndWantClickDir) {
							// bail out, we don't want it
							bCont = FALSE;
							break;
						}
						if (::GetParent(hwnd))
							hwnd = ::GetParent(hwnd);
					} while (::GetParent(hwnd));
					
					if (bCont) {
						int nphd2 = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
						HW_DATA *phd2 = nphd2 > 0 ? &pShData->hw[nphd2 - 1] : NULL;
						if (pShData->nWantClickDir == 2 || phd2 && (phd2->iClass == 5 || phd2->iClass == 9)) {
							GetCurrentWinDir(pShData->buf, phd2);
							if (pShDataCopy && pShDataCopy->hwndWantClickDir)
							{
								strcpy(pShDataCopy->buf, pShData->buf);
								Sleep(500);
							}
							SendMessage(pShData->hwndWantClickDir, WM_NULL, (WPARAM) WP_CURDIR_MSG, (LPARAM) LP_CURDIR_MSG);
						}
					}
				}
			}

#ifdef _DEBUG
			if (!nHotKey && pShData && pShData->m_bFindCtl && 
				(message == WM_LBUTTONDOWN || message == WM_NCLBUTTONDOWN))
			{
				TCHAR buf[MAX_PATH + 1], bufP[256];
				HWND hwnd = NULL, h, hwndParent;
				*buf = *bufP = 0;
				if (pM->hwnd) {
					POINT pt;
					pt.x = pM->pt.x;
					pt.y = pM->pt.y;
					hwnd = pM->hwnd; h = NULL;
					ScreenToClient(hwnd, &pt);
					while ((h = ChildWindowFromPointEx(hwnd, pt, CWP_ALL)) && h != hwnd) {
						hwnd = h;
						pt.x = pM->pt.x;
						pt.y = pM->pt.y;
						ScreenToClient(hwnd, &pt);
					}
					GetWindowText(hwnd, buf, 256);
				}
				int nID = GetDlgCtrlID(hwnd);
				int nIDParent;
				Msg(_T("\n\npM->hwnd = 0x%X,  Mouse on hwnd = 0x%X, nID = 0x%X, [%s]"), 
					pM->hwnd, hwnd, nID, buf);
				GetClassName(hwnd, buf, 256);
				Msg(_T(" -- Class name: %s, phd=%x, HandleThis=%d, iClass=%d"), buf, phd, (phd ? phd->bHandleThisWindow : 0), (phd ? phd->iClass : 0));
				
				//hwndParent = FindWindowEx(hwnd, NULL, _T("FloatNotifySink"), NULL);
				//Msg(" ---------- FloatNotifySink hwnd = 0x%X", hwndParent);

				hwndParent = hwnd;
				while ((hwndParent = GetAncestor(hwndParent, GA_PARENT)) != NULL)
				{
					nIDParent = GetDlgCtrlID(hwndParent);
					GetClassName(hwndParent, buf, 256);
					GetWindowText(hwndParent, bufP, 256);
					Msg(_T(" -- parent = 0x%X, nID = 0x%X, [%s]"), hwndParent, nIDParent, bufP);
					Msg(_T("    class: %s"), buf);
				}
				//LRESULT lRet = SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pM->pt.x, pM->pt.x));
				//Msg(_T("WM_NCHITTEST=%d"), lRet); // HTCAPTION etc.

				hwnd = GetTopWindow(pM->hwnd);
				while (hwnd) {
					*buf = 0;
					GetWindowText(hwnd, buf, 256);
					buf[255] = 0;
					nID = GetDlgCtrlID(hwnd);
					GetClassName(hwnd, bufP, 256);
					Msg(_T(" -- Child hwnd = 0x%X, ID = 0x%X, [%s], class [%s]"), hwnd, nID, buf, bufP);
					hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
				}

				hwnd = pM->hwnd;
				while (::GetParent(hwnd))
					hwnd = ::GetParent(hwnd);
				int nphd2 = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
				HW_DATA *phd2 = nphd2 > 0 ? &pShData->hw[nphd2 - 1] : NULL;
				GetCurrentWinDir(buf, phd2);
				Msg(_T("Cur dir: %s"), buf);
				
				pShData->m_bFindCtl = FALSE;
				pMsg->message = WM_NULL; // To disable processing
				return TRUE;
			}
#endif
			if (phd && phd->dwRelTime && GetTickCount() - phd->dwRelTime > 100) {
				if (phd->nBtnType[phd->iPushed - 1] <= 2) // only for menu buttons
					phd->iPushed = 0;
				phd->dwRelTime = 0;
			}
		}
	}

	LRESULT lRes = ( CallNextHookEx(hhkMouse, nCode, wParam0, lParam0));

	return lRes;
}

DF_SHARED_DATA * CALLBACK InstallMsgHooks(HINSTANCE hInstDLL, DWORD dwThreadId)
{
	HOOKPROC hHookProc;
	if (!hhkcb) {
		pShData->miscFlags1 &= ~1;
		RegisterBarWndClass();
		if (g_VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
			g_VerInfo.dwMajorVersion == 4) 
		{
			hHookProc = (HOOKPROC)GetProcAddress(hInstDLL, "DebugProc");
			hDebugHook = SetWindowsHookEx(WH_DEBUG, 
				(HOOKPROC) hHookProc, hInstDLL, 0);

		}

		hHookProc = (HOOKPROC)GetProcAddress(hInstDLL, "CallWndRetProc");
		hhkcb = SetWindowsHookEx(WH_CALLWNDPROCRET, 
			(HOOKPROC) hHookProc, hInstDLL, 0);
		/*
		ERROR_INVALID_HOOK_FILTER: 1426 The hook code is invalid.
		ERROR_INVALID_FILTER_PROC: 1427 The filter function is invalid.
		ERROR_HOOK_NEEDS_HMOD: 1428 A global hook is being set with a NULL hInstance parameter 
			or a thread-specific hook is being set for a thread that is not in the setting 
			application.
		ERROR_GLOBAL_ONLY_HOOK: 1429 A hook that can only be a system hook is being installed 
			to a specific thread.
		ERROR_INVALID_PARAMETER: 87 The thread ID is invalid.
		ERROR_JOURNAL_HOOK_SET: 1430 There is already a hook set for a journal hook type. 
			Only one journal record or journal playback hook can be installed at one time. 
			This code can also be set if an application tries to set a journal hook while 
			a screen saver is running.
		ERROR_MOD_NOT_FOUND: 126 The hInstance parameter for a global hook was not a library. 
			(Actually, this value simply means that User was unable to locate the module 
			handle in its list of modules.)
		Any other value: Security does not allow this hook to be set, or the system is out 
		of memory. 
		if (hhkcb == NULL) {
			dwErr = GetLastError();
			wsprintf(dbgBuf, "InstallMsgHooks: Error = 0x%X (%d)\n", dwErr, dwErr);
			OutputDebugString(dbgBuf);
		}
		wsprintf(dbgBuf, "InstallMsgHooks: hhkcb = 0x%X\n", hhkcb);
		OutputDebugString(dbgBuf);
		*/

		HOOKPROC hHookProc = (HOOKPROC)GetProcAddress(hInstDLL, "GetMsgProc");
		hhkMouse = SetWindowsHookEx( WH_GETMESSAGE ,        // install mouse message hook
									GetMsgProc,     // address of mouse hook procedure
									hInstDLL,			// handle to dll instance
									0 );	// hook procedure is associated with all existing threads. 
		DWORD_PTR dwResult = 0;
		SendMessageTimeout(HWND_BROADCAST,
			UM_FbxStartStop, 1, 0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			1000,
			(PDWORD_PTR) &dwResult);
	}
	
	return pShData;
}

DF_SHARED_DATA * CALLBACK GetShData()
{
	return pShData;
}

void CALLBACK ReleaseMsgHooks()
{
	while (pShData->iRecursion) {
		Sleep(1000);
	}
	//if (hhkcw)  
	//	UnhookWindowsHookEx(hhkcw);
	if (hDebugHook) {
		Msg(_T("Unhooking DebugProc"));
		if (!UnhookWindowsHookEx(hDebugHook))
			Msg(_T("  Error %d"), GetLastError());
		hDebugHook = NULL;
	}

	if (hhkcb) {
		DWORD_PTR dwResult = 0;

		pShData->miscFlags1 |= 1;
		RemoveLocalHooks();
		SendMessageTimeout(HWND_BROADCAST,
			UM_FbxStartStop, 0, 0,
			/*SMTO_ABORTIFHUNG | */SMTO_BLOCK,
			1000,
			(PDWORD_PTR) &dwResult);

		if (hhkMouse)
			UnhookWindowsHookEx(hhkMouse);
		hhkMouse = NULL;
		Msg(_T("Unhooking CallWndRetProc"));
		if (!UnhookWindowsHookEx(hhkcb))
			Msg(_T("  Error %d"), GetLastError());
		else
			hhkcb = NULL;

	}
 	

	UnRegisterBarWndClass();
	hhkMouse = NULL;
}


// This is a helper function for adding a menu item (either a popup 
// or command item) to the specified menu template.
//
//    MenuTemplate - pointer to a menu template
//    pcMenuString - string for the menu item to be added
//    MenuID       - id for the command item. Its value is ignored if 
//                   IsPopup is TRUE.
//    IsPopup      - TRUE for popup menu (or submenu); FALSE for command 
//                   item
//    LastItem     - TRUE if MenuString is the last item for the popup; 
//                   FALSE otherwise.
// For example on how to use see MSDN lib. disk, topic CMenu::LoadMenuIndirect 
//
UINT CALLBACK DFH_AddMenuItem(BYTE *MenuTemplate, const TCHAR *pcMenuString, 
							  WORD MenuID, BOOL IsPopup, BOOL LastItem)
{
	static BOOL bDoCrackCheck = TRUE;
    MENUEX_TEMPLATE_ITEM* mitem = (MENUEX_TEMPLATE_ITEM*) MenuTemplate;
	WCHAR MenuString[1024];

	memset(MenuString, 0, sizeof(MenuString));
#ifdef UNICODE
	strcpy(MenuString, pcMenuString);
#else
	MultiByteToWideChar(
	  CP_ACP,         // code page
	  MB_PRECOMPOSED,         // character-type options
	  pcMenuString, // address of string to map
	  -1,       // number of bytes in string
	  MenuString,  // address of wide-character buffer
	  1023        // size of buffer
	);
#endif

	UINT  bytes_used;
	bytes_used = sizeof(MENUEX_TEMPLATE_ITEM) - sizeof(WCHAR) - sizeof(DWORD);
	if (IsPopup)         // for popup menu
	{    
		mitem->dwType = MFT_STRING;
		mitem->dwState = MFS_ENABLED;
		mitem->uId = 0;
		if (LastItem)
			mitem->bResInfo = 0x81;
		else
			mitem->bResInfo = 0x01;
		wcscpy(mitem->szText, MenuString);
		bytes_used += sizeof (WCHAR) * ((unsigned int)wcslen(MenuString) + 1); // include '\0'

		if (bytes_used % 4 == 2) {
			MenuTemplate[bytes_used++] = 0;
			MenuTemplate[bytes_used++] = 0;
		}
		DWORD *pdw = (DWORD *) (MenuTemplate + bytes_used);
		*pdw = 0;

		bytes_used += sizeof (DWORD);  
	}
	else      // for command item
	{
		if (strstr(pcMenuString, _T("---")) == pcMenuString) {
			mitem->dwType = MFT_SEPARATOR;
			mitem->szText[0] = 0;
			bytes_used += sizeof(WCHAR);
		} else {
			mitem->dwType = MFT_STRING;
			wcscpy(mitem->szText, MenuString);
			bytes_used += sizeof (WCHAR) * ((unsigned int)wcslen(MenuString) + 1); // include '\0'
		}
		mitem->dwState = MFS_ENABLED;
		mitem->uId = MenuID;
		if (LastItem)
			mitem->bResInfo = 0x80;
		else
			mitem->bResInfo = 0x0;

		if (bytes_used % 4 == 2) {
			MenuTemplate[bytes_used++] = 0;
			MenuTemplate[bytes_used++] = 0;
		}
	}

	return bytes_used;
}

/* void MySendKeys(HWND hwnd, char *keys)
{
	int i, len = strlen(keys);
	int ch;
	INPUT in[2];
	BOOL bCapsWasPressed = FALSE;
	memset(&in[0], 0, sizeof(in));
	in[0].type = in[1].type = INPUT_KEYBOARD;
	in[1].ki.dwFlags = KEYEVENTF_KEYUP;

	if (GetAsyncKeyState(VK_CAPITAL) & 1)
	{
		bCapsWasPressed = TRUE;
		in[0].ki.wVk = in[1].ki.wVk = VK_CAPITAL;
		SendInput(2, in, sizeof(INPUT));
	}
	for (i = 0; i < len; i++)
	{
		ch = keys[i];
		int chu = ch;
		if (ch >= 'A' && ch <= 'Z') {
			in[0].ki.wVk = VK_SHIFT;
			SendInput(1, in, sizeof(INPUT));
		} else if (ch >= 'a' && ch <= 'z')
			chu -= ('a' - 'A');
		in[0].ki.wVk = in[1].ki.wVk = chu;
		SendInput(2, in, sizeof(INPUT));

		if (ch >= 'A' && ch <= 'Z') {
			in[1].ki.wVk = VK_SHIFT;
			SendInput(1, &in[1], sizeof(INPUT));
		}
	}
	if (bCapsWasPressed)
	{
		in[0].ki.wVk = in[1].ki.wVk = VK_CAPITAL;
		SendInput(2, in, sizeof(INPUT));
	}
}
*/

//LRESULT CALLBACK CallWndProc(
//  int nCode,       // hook code
//  WPARAM wParam,  // removal option
//  LPARAM lParam   // message
//)
//{
//	if (nCode >= 0 && pShData) {
//	}
//
//	return CallNextHookEx(
//		NULL,	// handle to current hook
//		nCode,	// hook code passed to hook procedure
//		wParam,	// value passed to hook procedure
//		lParam 	// value passed to hook procedure
//	   );
//}
