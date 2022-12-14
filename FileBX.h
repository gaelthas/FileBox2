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

// DFolder.h : main header file for the DFOLDER application
//

#if !defined(AFX_DFOLDER_H__B20AAE06_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
#define AFX_DFOLDER_H__B20AAE06_1F42_11D3_8563_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#ifndef MSG_FUN_DEFINED
#define MSG_FUN_DEFINED
#ifdef _DEBUG
  void Msg( LPCTSTR fmt, ... );
#else
  inline void Msg( LPCTSTR fmt, ... ) {}
#endif // _DEBUG
#endif


/////////////////////////////////////////////////////////////////////////////
// CDFolderApp:
// See DFolder.cpp for the implementation of this class
//

class CDFolderApp : public CWinApp
{
public:
	CDFolderApp();

	// Other
	BOOL	m_bAddToUninstall;	// Add user profile dir to install.log for removal
	CString m_sProgDir;			// Directory where FbX is installed, with trailing backslash
    DWORD MajorVersion, MinorVersion, BugFixVersion, BetaVersion;

	void RegPutString(const TCHAR *sSecName, const TCHAR *sKeyName, const TCHAR *sValue);
	CString RegGetString(const TCHAR *sSecName, const TCHAR *sKeyName, const TCHAR *sDefault = _T(""));

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDFolderApp)
	public:
	virtual BOOL InitInstance();

	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDFolderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CDFolderApp theApp;
extern CString g_sBrowseDir;
extern HINSTANCE hInstDLL;

HGLOBAL CreateHDrop(int cnt, TCHAR *names[]);

HRESULT CreateShortCut(LPCTSTR pszTargetFile, LPCTSTR pszLink, LPTSTR pszDesc = NULL);
HRESULT ResolveShortCut(HWND hwnd, LPCTSTR pszShortcutFile, 
						WIN32_FIND_DATA &wfd, LPTSTR pszPath = NULL,
						LPTSTR pszDescription = NULL, BOOL bDoResolve = FALSE,
						LPTSTR pszWorkDir = NULL,
						LPTSTR pszArgs = NULL,
						int   *piShowCmd = NULL
						);
HRESULT SetItemNumber(HWND hwnd, LPCTSTR pszShortcutFile, int nItemNo);
BOOL IsFolder(const TCHAR *pszFile);
BOOL MoveFileOrFolder(const TCHAR *szSrc, const TCHAR *szTarget);
UINT AddMenuItem(BYTE *MenuTemplate, const TCHAR *pcMenuString, WORD MenuID, BOOL IsPopup, BOOL LastItem);
CString EncodeFileName(const TCHAR *inStr);
CString DecodeFileName(const TCHAR *inStr);
BOOL FillRecentFoldersBuffer();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DFOLDER_H__B20AAE06_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
