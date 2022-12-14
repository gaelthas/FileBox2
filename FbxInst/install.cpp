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
#include "resource.h"
#include "../DFH/DFH_shared.h"

// This is wrong here!
//#pragma data_seg(".MYSEC")
//BYTE fbxSharedData[SHMEMSIZE] = {0, 0, 0, 0}; // MUST BE INITIALIZED TO BE IN SHARED SEGMENT!
//#pragma data_seg()
//#pragma comment(linker, "/SECTION:.MYSEC,RWS")

TCHAR szVerStr[64];
HBRUSH hBrush = NULL;
HCURSOR hHand;

extern "C" {
__declspec( dllexport ) BOOL CALLBACK FbXInstallSupport();
BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved);
}


BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	return(1);
}

BOOL CALLBACK ErrorDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	RECT rct;
	int w, h;

	switch (uMsg) {
	case WM_INITDIALOG:
		::GetWindowRect(hwndDlg, &rct);
		w = ::GetSystemMetrics(SM_CXSCREEN);
		h = ::GetSystemMetrics(SM_CYSCREEN);
		w = (w - rct.right + rct.left)/2;
		h = (h - rct.bottom + rct.top)/2;
		::SetWindowPos(hwndDlg, HWND_TOP, w, h, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		::SetWindowText(::GetDlgItem(hwndDlg, IDC_VERSTR), szVerStr);
		break;

	case WM_COMMAND:
		EndDialog(hwndDlg, 0);
		break;

	}

	return FALSE;
}


BOOL CALLBACK FbXInstallSupport()
{
	BOOL bNeedUpdate = FALSE;
	// Check Version of ComCtl32.dll
	DWORD dwSize, dw;
	TCHAR fname[MAX_PATH];
	GetSystemDirectory(fname, MAX_PATH);
	strcat(fname, _T("\\ComCtl32.dll"));
	dwSize = GetFileVersionInfoSize(fname, &dw);
	if (dwSize) {
		void *pVer = malloc(dwSize);
		VS_FIXEDFILEINFO  *pVerInfo;
		UINT len;
		if (GetFileVersionInfo(fname, dw, dwSize, pVer)) {
			VerQueryValue(pVer, TEXT("\\"), (void **) &pVerInfo, &len); 
			ULONGLONG ull = ((ULONGLONG) pVerInfo->dwFileVersionMS) << 32;
			ull |= pVerInfo->dwFileVersionLS;

			ULONGLONG ullNeed = ((ULONGLONG) 4 << 48) | ((ULONGLONG) 72 << 32) | 3611 | 1900;
						
			if (ull < ullNeed)
//			if (ull >= ullNeed) // for test only to see error message
			{
				wsprintf(szVerStr, 
					_T("%d.%d.%d.%d"),
					(UINT) (ull >> 48),
					(UINT) (ull >> 32) & 0xffff,
					(UINT) (ull >> 16) & 0xffff,
					(UINT) (ull) & 0xffff
				);
				bNeedUpdate = TRUE;
			}
		}
		free(pVer);
	}

	if (bNeedUpdate) {
		hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
		hHand = ::LoadCursor(GetModuleHandle(_T("FbxInst.dll")), MAKEINTRESOURCE(IDC_FBX_HAND));

		int iRet = DialogBox(
			  GetModuleHandle(_T("FbxInst.dll")),  // handle to module
			  MAKEINTRESOURCE(IDD_DIALOG1),   // dialog box template
			  NULL,      // handle to owner window
			  (DLGPROC) ErrorDialogProc  // dialog box procedure
			);
		::DeleteObject(hBrush);

		return TRUE; // Will exit installation
	}

	UINT FBX_QUIT =  RegisterWindowMessage("FileBox eXtender Quit Message");
	PostMessage(HWND_BROADCAST, FBX_QUIT, 0, 0);
	Sleep(1000);
	DWORD_PTR dwResult = 0;
	SendMessageTimeout(HWND_BROADCAST,
		WM_NCPAINT, 1, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL,
		1000,
		&dwResult);
	Sleep(1000);

	return FALSE; // Will continue installation
}