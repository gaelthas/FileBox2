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

// DFolder.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FileBX.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "OptionsDlg.h"
#include "KeySetDlg.h"
#include "FileBxDlg.h"
#include "DFH/DFH_shared.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _WIN64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' \
version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef BroadcastSystemMessage
  // for missing BroadcastSystemMessageA in Win95
# undef BroadcastSystemMessage
#endif

typedef DF_SHARED_DATA * (CALLBACK *GetShData_t) ();
GetShData_t GetShData = NULL;

#ifdef _DEBUG

void Msg( LPCTSTR fmt, ... )
{
    static TCHAR    buff[256];
    va_list  va;

    va_start(va, fmt);

    //
    // format message with header
    //
    //strcpy( buff, _T("HSDX: ") );
    //vsprintf( &buff[strlen(buff)], fmt, va );
    vsprintf( buff, fmt, va );
    strcat( buff, _T("\r\n"));

    OutputDebugString( buff );
} /* Msg */

#endif _DEBUG


/////////////////////////////////////////////////////////////////////////////
// CDFolderApp

BEGIN_MESSAGE_MAP(CDFolderApp, CWinApp)
	//{{AFX_MSG_MAP(CDFolderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
	//ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDFolderApp construction

CDFolderApp::CDFolderApp()
{
	// Place all significant initialization in InitInstance
	m_bAddToUninstall = FALSE;

	// Check Version of our EXE file
	DWORD dwSize, dw;
	TCHAR fname[_MAX_PATH + 20];
	GetModuleFileName(NULL, fname, _MAX_PATH + 20);
	dwSize = GetFileVersionInfoSize(fname, &dw);
	if (dwSize) {
		void *pVer = malloc(dwSize);
		VS_FIXEDFILEINFO  *pVerInfo;
		UINT len;
		if (GetFileVersionInfo(fname, dw, dwSize, pVer)) {
			VerQueryValue(pVer, TEXT("\\"), (void **) &pVerInfo, &len);
			MajorVersion = pVerInfo->dwFileVersionMS >> 16;
			MinorVersion = pVerInfo->dwFileVersionMS & 0xffff;
			BugFixVersion = pVerInfo->dwFileVersionLS >> 16;
			BetaVersion = pVerInfo->dwFileVersionLS & 0xffff;
		}
		free(pVer);
	}

	TCHAR *pc = fname + strlen(fname) - 1;
	while (pc >= fname && *pc != '\\' && *pc != '/' && *pc != ':')
		pc--;
	*pc = 0;
	m_sProgDir = fname + CString("\\");
}

void CDFolderApp::RegPutString(const TCHAR *sSecName, const TCHAR *sKeyName, const TCHAR *sValue)
{
	AfxGetApp()->WriteProfileString(sSecName, sKeyName, sValue);
}

CString CDFolderApp::RegGetString(const TCHAR *sSecName, const TCHAR *sKeyName, const TCHAR *sDefault)
{
	return AfxGetApp()->GetProfileString(sSecName, sKeyName, sDefault);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDFolderApp object

CDFolderApp theApp;

#define STATIC_LIB
#define CRYPTLIB_LITE
#undef _WINDOWS

/////////////////////////////////////////////////////////////////////////////
// CDFolderApp initialization

BOOL CDFolderApp::InitInstance()
{
	HINSTANCE hInst = ::LoadLibrary(_T("FbxRes.dll"));
	if (hInst != NULL)
        AfxSetResourceHandle(hInst);
	else {
		DWORD dwError = ::GetLastError();
		TCHAR s[256];
		sprintf(s, _T("FbxRes.dll not found (err %d, 0x%X), exiting."), dwError, dwError);
		AfxMessageBox(s);
		return FALSE;
	}
	InitCommonControls();
	//if (m_VerInfo.dwMajorVersion > 5)
	//{
	//	::AfxMessageBox("Sorry, FileBox eXtender can only be used under Windows XP or older versions of Windows."
	//					"This program will now exit.");
	//	return FALSE;
	//}

	DWORD dwTestLic = 1; // must be 1 to display "favorites", if testing lic. not needed.
	{
		TCHAR fname[MAX_PATH + 20], *pc;
		strcpy(fname, theApp.m_pszHelpFilePath);
		for (pc = fname + strlen(fname); *pc != '\\' && pc > fname; pc--);
		if (*pc == '\\') pc++;
		*pc = 0;
		theApp.m_sProgDir = fname;

		strlwr((TCHAR*) theApp.m_pszHelpFilePath);
		pc = strstr((TCHAR *)theApp.m_pszHelpFilePath, _T(".hlp"));
		if (pc)
			strcpy(pc, _T(".chm"));

	}

	// Initialize OLE libraries
	if (!AfxOleInit()) // need them for drag and drop!
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MFC_VER < 0x0700
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif

	SetRegistryKey(_T("Hyperionics"));

#ifdef _WIN64
	/* Create a named file mapping object */ 
	HANDLE hMapObject = NULL;   /* handle to file mapping */ 

    hMapObject = CreateFileMapping( 
        (HANDLE) INVALID_HANDLE_VALUE, /* use paging file    */ 
        NULL,                /* no security attr.  */ 
        PAGE_READWRITE,      /* read/write access  */ 
        0,                   /* size: high 32-bits */ 
        SHMEMSIZE_COPY,				 /* size: low 32-bits  */ 
        _T("DFolderHook64-32"));      /* name of map object */ 
    /* The first process to attach initializes memory. */ 
    BOOL fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 

    if (hMapObject == NULL) {
		TCHAR s[256];
		sprintf(s, _T("CreateFileMapping() failed, error no. %d"), GetLastError());
		AfxMessageBox(s);
        return FALSE; 
	}

    /* Get a pointer to the file-mapped shared memory. */ 
    pShDataCopy = (DF_SHARED_DATA *) MapViewOfFile( 
        hMapObject,     /* object to map view of    */ 
        FILE_MAP_WRITE, /* read/write access        */ 
        0,              /* high offset:   map from  */ 
        0,              /* low offset:    beginning */ 
        0);             /* default: map entire file */ 
    if (pShDataCopy == NULL) {
		TCHAR s[256];
		sprintf(s, _T("MapViewOfFile() failed, error no. %d"), GetLastError());
		AfxMessageBox(s);
        return FALSE; 
	}
	if (fInit)
	{
		memset(pShDataCopy, '\0', SHMEMSIZE_COPY);
		pShDataCopy->dwSize = SHMEMSIZE;
	}
#endif

	/* Initialize memory if this is the first process. */ 
	BOOL bDoQuit = (__argc > 1 && strcmp(__argv[1], _T("-quit")) == 0);
	if (!hInstDLL)
		hInstDLL = LoadLibrary(_T("FileBXH.dll"));
	if (hInstDLL)
		GetShData = (GetShData_t) GetProcAddress(hInstDLL, "GetShData"); 
	if (GetShData == NULL) {
		::AfxMessageBox(_T("Error, FileBXH.dll not found or wrong version."));
		return 1;
	}
	pShData = GetShData();

    if (pShData->dwSize == 0) 
	{
		if (bDoQuit) {
			DWORD_PTR dwResult = 0;
			SendMessageTimeout(HWND_BROADCAST,
				WM_NCPAINT, 1, 0,
				SMTO_ABORTIFHUNG | SMTO_NORMAL,
				1000,
				(PDWORD_PTR) &dwResult);
			Sleep(1000);
		} else {
			extern int ReadIcoToResource(TCHAR *fname, BYTE *pBytes, int nMaxSize);

			Msg(_T("Initializing shared memory. "));
			memset(pShData, '\0', SHMEMSIZE);
			pShData->dwSize = SHMEMSIZE;
			//CString s;
			//s.Format("FbX %d bit: sizeof(DF_SHARED_DATA) = %d\n", 8*sizeof(HWND), sizeof(DF_SHARED_DATA));
			//OutputDebugString(s);
			
			TCHAR sDir[MAX_PATH + 20], *pc;
			GetModuleFileName(NULL, sDir, MAX_PATH);
			for (pc = sDir + strlen(sDir); pc > sDir && *pc != '\\'; pc--);

			strcpy(pc, _T("\\favorite.ico"));
			ReadIcoToResource(sDir, pShData->bIconBits[0], MAX_ICON_SIZE);
			strcpy(pc, _T("\\recent.ico"));
			ReadIcoToResource(sDir, pShData->bIconBits[1], MAX_ICON_SIZE);
			
			if (__argc > 2 && strcmp(__argv[1], _T("-cf")) == 0) 
			{
				extern CString g_sCfgDir;
				g_sCfgDir = __argv[2];
			} 
		}

	} 
	else if (pShData->hDFhWnd && IsWindow(pShData->hDFhWnd)) 
	{
		if (bDoQuit) {
			PostMessage(pShData->hDFhWnd, WM_QUIT, 0, 0);
			Sleep(1000);
			DWORD_PTR dwResult = 0;
			SendMessageTimeout(HWND_BROADCAST,
				WM_NCPAINT, 1, 0,
				SMTO_ABORTIFHUNG | SMTO_NORMAL,
				1000,
				&dwResult);
			Sleep(1000);

		} 
		else if (__argc > 2 && strcmp(__argv[1], _T("-a")) == 0) 
		{
			strcpy(pShData->buf, pShData->szAdd);
			strcpy(pShData->buf + strlen(pShData->buf) + 1, __argv[2]);
			PostMessage(pShData->hDFhWnd, WM_COMMAND, ID_EDIT_FV, 9);
		} 
		else
		{
			PostMessage(pShData->hDFhWnd, WM_COMMAND, ID_EDIT_FV, dwTestLic);
		}

#ifdef _WIN64
		CloseHandle(hMapObject);
		hMapObject = NULL;
#endif
		bDoQuit = TRUE;
		
	}

	if (bDoQuit) 
	{
		//if (hMapObject) {
		//	CloseHandle(hMapObject);
		//	pShData = NULL;
		//}
		return FALSE;
	}

	/*
	CTime ct = CTime::GetCurrentTime();
    if ((ct.GetMonth() < 10 || ct.GetMonth() > 11 || ct.GetYear() != 1999)) {
		CString s;
		s.LoadString(IDS_BETA_EXPIRED);
        AfxMessageBox(s);
        bDoQuit = TRUE;
    }
	*/

	MSG msg;
	CFileBxDlg dlg;
	m_pMainWnd = &dlg;

	dlg.Create(CFileBxDlg::IDD);
	dlg.ShowWindow(__argc > 1 && strcmp(__argv[1], _T("-v")) == 0 ? theApp.m_nCmdShow : SW_HIDE);
	if (pShData)
		pShData->hDFhWnd = dlg.m_hWnd;


	if (pShDataCopy && pShData->dwSize == pShDataCopy->dwSize)
		memcpy(pShDataCopy, pShData, SHMEMSIZE_COPY);
#ifdef _WIN64
	// Start Fbx32helper.exe
	CString mn;		
	GetModuleFileName(NULL, mn.GetBuffer(MAX_PATH+1), MAX_PATH);
	mn.ReleaseBuffer();
	mn = mn.Left(mn.ReverseFind('\\') + 1) + "Fbx32helper.exe";
	WinExec(mn, SW_HIDE);
#endif

	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!IsDialogMessage(dlg.m_hWnd, &msg))
			DispatchMessage(&msg);
	}

	dlg.SaveStatus();

	if (pShData)
		pShData->hDFhWnd = NULL;

	if (dlg.IsHookInstalled())
		dlg.ToggleHooks();

	dlg.DestroyWindow();

	//  if (hMapObject) {
	//  CloseHandle(hMapObject);
	//	pShData = NULL;
	//}

	// To unload our DLL...
	DWORD_PTR dwResult = 0;
	SendMessageTimeout(HWND_BROADCAST,
		WM_NCPAINT, 1, 0,
		SMTO_ABORTIFHUNG | SMTO_NORMAL,
		1000,
		&dwResult);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

