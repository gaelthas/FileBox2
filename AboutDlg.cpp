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

// AboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "AboutDlg.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "OptionsDlg.h"
#include "KeySetDlg.h"
#include "FileBxDlg.h"

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_sCfgFolder = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Control(pDX, IDC_VERSION, m_cVersion);
	DDX_Control(pDX, IDC_HTTP, m_cHTTP);
	DDX_Control(pDX, IDC_EMAIL, m_cEmail);
	DDX_Text(pDX, IDC_CFG_FOLDER, m_sCfgFolder);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_GNU_LICENSE, m_cLicInfo);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONTRIBUTORS, &CAboutDlg::OnBnClickedContributors)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    TCHAR buf[128];
    CString sText;
	
	m_cVersion.GetWindowText(sText);
    sprintf(buf, sText, theApp.MajorVersion, theApp.MinorVersion, theApp.BugFixVersion, 8*sizeof(HANDLE),
		sizeof(TCHAR) > 1 ? _T(", UNICODE") : _T(""));
	if (theApp.BetaVersion)
		sprintf(buf + strlen(buf), _T(" Beta %d"), theApp.BetaVersion);

	m_cVersion.SetWindowText(buf);

	
	m_hHand = ::LoadCursor(AfxGetResourceHandle(),MAKEINTRESOURCE(IDC_FBX_HAND));
	m_sCfgFolder = pMainDlg->m_pFavDlg->m_cTree.m_pszFolder;
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAboutDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect r;
	m_cEmail.GetWindowRect(&r);
	ScreenToClient(&r);
	if (PtInRect(&r, point)) {
		CString s;
		m_cEmail.GetWindowText(s);
		::ShellExecute(m_hWnd, NULL, _T("mailto:") + s, NULL, 
			_T(".\\"), SW_SHOW);
	}

	m_cHTTP.GetWindowRect(&r);
	ScreenToClient(&r);
	if (PtInRect(&r, point)) {
		CString s;
		m_cHTTP.GetWindowText(s);
		::ShellExecute(m_hWnd, NULL, s, NULL, 
			_T(".\\"), SW_SHOW);
	}

	m_cLicInfo.GetWindowRect(&r);
	ScreenToClient(&r);
	if (PtInRect(&r, point)) {
		CString s = _T("file://") + theApp.m_sProgDir + _T("license.txt");
		::ShellExecute(m_hWnd, NULL, s, NULL, 
			_T(".\\"), SW_SHOW);
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CAboutDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect r1, r2, r3;
	CPoint pt(point);
	ClientToScreen(&pt);
	m_cEmail.GetWindowRect(&r1);
	m_cHTTP.GetWindowRect(&r2);
	m_cLicInfo.GetWindowRect(&r3);
	
	if (PtInRect(&r1, pt) || PtInRect(&r2, pt) || PtInRect(&r3, pt)) {
		::SetCursor(m_hHand);
	} else {
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CAboutDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect r1, r2;
	m_cEmail.GetWindowRect(&r1);
	ScreenToClient(&r1);
	m_cHTTP.GetWindowRect(&r2);
	ScreenToClient(&r2);
	CString s;
	m_cEmail.GetWindowText(s);

	CFont *pFontOrg = GetFont();
	LOGFONT lf;
	pFontOrg->GetLogFont(&lf);
	lf.lfUnderline = TRUE;
	CFont *pFont = new CFont;
	pFont->CreateFontIndirect(&lf);
	pFont = dc.SelectObject(pFont);
	dc.SetTextColor(0xFF0000);
	dc.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	dc.DrawText(s, &r1, DT_LEFT);

	m_cHTTP.GetWindowText(s);
	dc.DrawText(s, &r2, DT_LEFT);
	
	pFont = dc.SelectObject(pFont);
	pFont->DeleteObject();
	delete pFont;
	// Do not call CDialog::OnPaint() for painting messages
}

void CAboutDlg::OnBnClickedContributors()
{
	CString s = _T("file://") + theApp.m_sProgDir + _T("contributors.txt");
	::ShellExecute(m_hWnd, NULL, s, NULL, 
		_T(".\\"), SW_SHOW);
}
