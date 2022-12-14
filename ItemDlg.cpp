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

// ItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "ItemDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemDlg dialog


CItemDlg::CItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CItemDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CItemDlg)
	m_sDesc = _T("");
	m_sTarget = _T("");
	m_sItemType = _T("");
	//}}AFX_DATA_INIT
}


void CItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CItemDlg)
	DDX_Control(pDX, IDC_ITEM_TARGET, m_cItemTarget);
	DDX_Control(pDX, IDC_ITEM_DESC, m_cItemDesc);
	DDX_Control(pDX, IDOK, m_cOK);
	DDX_Text(pDX, IDC_ITEM_DESC, m_sDesc);
	DDX_Text(pDX, IDC_ITEM_TARGET, m_sTarget);
	DDX_Text(pDX, IDC_ITEM_TYPE, m_sItemType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CItemDlg, CDialog)
	//{{AFX_MSG_MAP(CItemDlg)
	ON_BN_CLICKED(IDC_BROWSE_FOLDER, OnBrowseFolder)
	ON_BN_CLICKED(IDC_BROWSE_FILE, OnBrowseFile)
	ON_EN_CHANGE(IDC_ITEM_DESC, OnChangeItemDesc)
	ON_EN_CHANGE(IDC_ITEM_TARGET, OnChangeItemTarget)
	ON_BN_CLICKED(IDC_HELP_ITPROP, OnHelpItprop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemDlg message handlers

BOOL CItemDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_hwndHelp = NULL;

	if (IsFolder(m_sTarget))
		m_sItemType.LoadString(IDS_SHORTCUT_FOLD);
	else if (_access(m_sTarget, 0) == 0)
		m_sItemType.LoadString(IDS_SHORTCUT_FILE);
	else
		m_sItemType.LoadString(IDS_TEXT_ITEM);

	UpdateData(FALSE);
	if (m_sTarget.IsEmpty() || m_sDesc.IsEmpty())
		m_cOK.EnableWindow(FALSE);
	else
		m_cOK.EnableWindow(TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg) {
    case BFFM_INITIALIZED: {
       if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir)) {
          // WParam is TRUE since you are passing a path.
          // It would be FALSE if you were passing a pidl.
          SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);
       }
       break;
    }
    case BFFM_SELCHANGED: {
       // Set the status window to the currently selected path.
       if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) {
          SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
       }
       break;
    }
    default:
       break;
	}
	return 0;
}


void CItemDlg::OnBrowseFolder() 
{
	BROWSEINFO bi;
	LPITEMIDLIST pItemList;
	TCHAR szDisplayName[MAX_PATH];
	memset(&bi, 0, sizeof(bi));
	CString sPrompt;
	sPrompt.LoadString(IDS_FOLDER_SELECT);

    bi.hwndOwner = m_hWnd; 
    bi.pidlRoot = NULL; 
    bi.pszDisplayName = szDisplayName; 
    bi.lpszTitle = sPrompt; 
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;

	/*
    LPARAM lParam; 
    int iImage; 
	*/
	TCHAR buf2[MAX_PATH + 20];
	GetCurrentDirectory(MAX_PATH, buf2);
	SetCurrentDirectory(g_sBrowseDir);
	if (pItemList = SHBrowseForFolder(&bi)) {
		TCHAR buf[MAX_PATH + 20];
		if (SHGetPathFromIDList(pItemList, buf)) {
			m_sDesc = szDisplayName;
			m_sTarget = buf;
			m_sItemType.LoadString(IDS_SHORTCUT_FOLD);
			UpdateData(FALSE);
			OnChangeItemTarget();
			g_sBrowseDir = m_sTarget;
			// SetCurrentDirectory(m_sTarget);
		}
		LPMALLOC pMalloc;
		if (SHGetMalloc(&pMalloc) == NOERROR) 
			pMalloc->Free(pItemList);
	}
	SetCurrentDirectory(buf2);
}

void CItemDlg::OnBrowseFile() 
{
	OPENFILENAME ofn;
	TCHAR buf[2*MAX_PATH + 20], szFileName[2*MAX_PATH + 20];
	*buf = 0;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = _T("*.*\0*.*\0");
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = szFileName;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL; // Current directory
	ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn)) {
		m_sTarget = buf;
		m_sDesc = szFileName;
		if (IsFolder(buf))
			m_sItemType.LoadString(IDS_SHORTCUT_FOLD);
		else if (_access(buf, 0) == 0)
			m_sItemType.LoadString(IDS_SHORTCUT_FILE);
		else
			m_sItemType.LoadString(IDS_TEXT_ITEM);
		UpdateData(FALSE);
		OnChangeItemTarget();
	}
}

void CItemDlg::OnChangeItemDesc() 
{
	CString s1, s2;
	m_cItemDesc.GetWindowText(s1);
	m_cItemTarget.GetWindowText(s2);
	if (s1.IsEmpty() || s2.IsEmpty())
		m_cOK.EnableWindow(FALSE);
	else
		m_cOK.EnableWindow(TRUE);
}

void CItemDlg::OnChangeItemTarget() 
{
	CString s1, s2;
	m_cItemDesc.GetWindowText(s1);
	m_cItemTarget.GetWindowText(s2);
	if (s1.IsEmpty() || s2.IsEmpty())
		m_cOK.EnableWindow(FALSE);
	else
		m_cOK.EnableWindow(TRUE);
}


void CItemDlg::OnHelpItprop() 
{
	m_hwndHelp = ::HtmlHelp(m_hWnd, CString(theApp.m_pszHelpFilePath) + "::/html/item_props.html>popup", HH_DISPLAY_TOPIC, NULL);	
	//theApp.WinHelp(IDD_NEW_ITEM + 0x20000, HELP_CONTEXT);	
}

BOOL CItemDlg::DestroyWindow() 
{
	if (m_hwndHelp != NULL && ::IsWindow(m_hwndHelp))
		::SendMessage(m_hwndHelp, WM_CLOSE, 0, 0);
	
	return CDialog::DestroyWindow();
}
