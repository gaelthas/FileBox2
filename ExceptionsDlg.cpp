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

// ExceptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "OptionsDlg.h"
#include "KeySetDlg.h"
#include "FileBxDlg.h"
#include "ExceptionsDlg.h"
#include "DFH/DFH_shared.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExceptionsDlg dialog

static char *szChoices = "0nhv";

CExceptionsDlg::CExceptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExceptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExceptionsDlg)
	//}}AFX_DATA_INIT
}


void CExceptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExceptionsDlg)
	DDX_Control(pDX, IDC_EX_NOROLL, m_cNoRoll);
	DDX_Control(pDX, IDC_EX_VRESIZE, m_cVResize);
	DDX_Control(pDX, IDC_EX_HRESIZE, m_cHResize);
	DDX_Control(pDX, IDC_EX_FOLDERBTNS, m_cFoldersBtn);
	DDX_Control(pDX, IDC_EX_NOPIN, m_cNoPin);
	DDX_Control(pDX, IDC_EX_DONTHANDLE, m_cDontHandle);
	DDX_Control(pDX, IDC_DEL_EXCEPT, m_cDelExcBtn);
	DDX_Control(pDX, IDC_EXC_LIST, m_cExcList);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_MOVE_LEFT, m_cMoveLeft);
}


BEGIN_MESSAGE_MAP(CExceptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CExceptionsDlg)
	ON_LBN_SELCHANGE(IDC_EXC_LIST, OnSelchangeExcList)
	ON_BN_CLICKED(IDC_ADD_EXCEPT, OnAddExcept)
	ON_BN_CLICKED(IDC_DEL_EXCEPT, OnDelExcept)
	ON_BN_CLICKED(IDC_EX_DONTHANDLE, OnExDonthandle)
	ON_BN_CLICKED(IDC_EX_NOPIN, OnExNopin)
	ON_BN_CLICKED(IDC_EX_FOLDERBTNS, OnExFolderbtns)
	ON_BN_CLICKED(IDC_EX_HRESIZE, OnExHresize)
	ON_BN_CLICKED(IDC_EX_VRESIZE, OnExVresize)
	ON_BN_CLICKED(IDC_EX_NOROLL, OnExNoroll)
	//}}AFX_MSG_MAP
	ON_EN_KILLFOCUS(IDC_MOVE_LEFT, &CExceptionsDlg::OnEnKillfocusMoveLeft)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExceptionsDlg message handlers

BOOL CExceptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	for (int i = 0; i < pShData->nExceptions; i++) 
	{
		const char *pc = pShData->sExceptions[i];
		const char *pc0 = strstr(pc, " \\");
		if (pc0)
			pc0++;
		else if (pc0 = strstr(pc, ":\\"))
			pc0--;
		if (pc0)
		{
			m_cExcList.AddString(pc0);
			DWORD nEx = (HexDigit(pc[0])<<12) + (HexDigit(pc[1])<<8) + (HexDigit(pc[2])<<4) + HexDigit(pc[3]);
			m_cExcList.SetItemData(i, nEx);
		}
	}
	m_cExcList.SetCurSel(0);
	OnSelchangeExcList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExceptionsDlg::OnSelchangeExcList() 
{
	int n = m_cExcList.GetCurSel();
	/*
	CWnd *pWnd;
	for (i = IDC_EX_RADIO2; i <= IDC_EX_RADIO4; i++) {
		pWnd = GetDlgItem(i);
		pWnd->EnableWindow(n > -1);
	}
	*/
	m_cDelExcBtn.EnableWindow(n > -1);

	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n)>>8;
		m_cDontHandle.SetCheck(nEx & 1);
		m_cNoPin.SetCheck(nEx & FB_NO_PUSHPIN);
		m_cNoRoll.SetCheck(nEx & FB_NO_ROLLUP);
		m_cHResize.SetCheck(nEx & FB_RESIZE_HONLY);
		m_cVResize.SetCheck(nEx & FB_RESIZE_VONLY);
		m_cFoldersBtn.SetCheck(nEx & FB_NO_FBTNS);

		nEx = m_cExcList.GetItemData(n) & 0xff;
		char s[20];
		itoa(nEx, s, 10);
		m_cMoveLeft.SetWindowText(s);
		UpdateData(FALSE);
	}
	OnExDonthandle();
}

void CExceptionsDlg::OnAddExcept() 
{
	OPENFILENAME ofn;
	CString sProgFiles, sDllFiles, sAllFiles;
	TCHAR buf[2*MAX_PATH + 20], szFileName[2*MAX_PATH + 20];
	*buf = 0;
	memset(&ofn, 0, sizeof(ofn));
	sProgFiles.LoadString(IDS_PROG_FILES);
	sDllFiles.LoadString(IDS_DLL_FILES);
	sAllFiles.LoadString(IDS_ALL_FILES);
	TCHAR szFilter[256];
	sprintf(szFilter, _T("%s (*.exe)#*.exe#%s (*.dll)#*.dll#%s (*.*)#*.*#"), sProgFiles, sDllFiles, sAllFiles);
	int i, len = (int)strlen(szFilter);
	for (i = 0; i < len; i++)
		if (szFilter[i] == '#')
			szFilter[i] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = szFilter;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = szFileName;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = NULL; // Current directory
	ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn)) {
		strlwr(buf);
		DWORD n = m_cExcList.AddString(buf);
		m_cExcList.SetItemData(n, (FB_RESIZE_HONLY | FB_RESIZE_VONLY) << 8);
		m_cExcList.SetCurSel(n);
		OnSelchangeExcList();
		OnOK();
	}
}

void CExceptionsDlg::OnDelExcept() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		m_cExcList.DeleteString(n);
		OnSelchangeExcList();
		OnOK();
	}
}


void CExceptionsDlg::OnOK() 
{
	for (int i = 0; i < m_cExcList.GetCount(); i++) {
		CString s;
		m_cExcList.GetText(i, s);
		sprintf(pShData->sExceptions[i], _T("%04x %s"), m_cExcList.GetItemData(i), s);
		strlwr(pShData->sExceptions[i]);
	}
	pShData->nExceptions = m_cExcList.GetCount();
	if (pShDataCopy && pShData->dwSize == pShDataCopy->dwSize)
		memcpy(pShDataCopy, pShData, SHMEMSIZE_COPY);
	
	// CDialog::OnOK(); // no more! it's a tab now!
}


void CExceptionsDlg::OnExDonthandle() 
{
	int nShow = m_cDontHandle.GetCheck() ? SW_HIDE : SW_SHOW;
	m_cVResize.ShowWindow(nShow);
	m_cHResize.ShowWindow(nShow);
	m_cFoldersBtn.ShowWindow(nShow);
	m_cNoPin.ShowWindow(nShow);
	m_cNoRoll.ShowWindow(nShow);

	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (nShow)
			nEx &= ~(1<<8);
		else
			nEx |= 1<<8;
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}

}

void CExceptionsDlg::OnExNopin() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (m_cNoPin.GetCheck())
			nEx |= FB_NO_PUSHPIN<<8;
		else
			nEx &= ~(FB_NO_PUSHPIN<<8);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}

void CExceptionsDlg::OnExNoroll() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (m_cNoRoll.GetCheck())
			nEx |= FB_NO_ROLLUP<<8;
		else
			nEx &= ~(FB_NO_ROLLUP<<8);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}


void CExceptionsDlg::OnExFolderbtns() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (m_cFoldersBtn.GetCheck())
			nEx |= FB_NO_FBTNS<<8;
		else
			nEx &= ~(FB_NO_FBTNS<<8);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}

void CExceptionsDlg::OnExHresize() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (m_cHResize.GetCheck())
			nEx |= FB_RESIZE_HONLY<<8;
		else
			nEx &= ~(FB_RESIZE_HONLY<<8);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}

void CExceptionsDlg::OnExVresize() 
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		if (m_cVResize.GetCheck())
			nEx |= FB_RESIZE_VONLY<<8;
		else
			nEx &= ~(FB_RESIZE_VONLY<<8);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}


void CExceptionsDlg::OnEnKillfocusMoveLeft()
{
	int n = m_cExcList.GetCurSel();
	if (n > -1) {
		DWORD nEx = m_cExcList.GetItemData(n);
		nEx &= 0xff00;
		CString s;
		m_cMoveLeft.GetWindowTextA(s);
		nEx |= atoi(s);
		m_cExcList.SetItemData(n, nEx);
		OnOK();
	}
}
