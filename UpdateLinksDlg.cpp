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

// UpdateLinksDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "UpdateLinksDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdateLinksDlg dialog


CUpdateLinksDlg::CUpdateLinksDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateLinksDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdateLinksDlg)
	m_iUpdateMethod = -1;
	//}}AFX_DATA_INIT
}


void CUpdateLinksDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdateLinksDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_iUpdateMethod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdateLinksDlg, CDialog)
	//{{AFX_MSG_MAP(CUpdateLinksDlg)
	ON_BN_CLICKED(ID_HELP_UPLINKS, OnHelpUplinks)
	//}}AFX_MSG_MAP
//	ON_BN_CLICKED(IDC_RADIO1, &CUpdateLinksDlg::OnBnClickedRadio1)
//ON_BN_CLICKED(IDOK, &CUpdateLinksDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdateLinksDlg message handlers

BOOL CUpdateLinksDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_iUpdateMethod = 0;
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdateLinksDlg::OnOK() 
{
	UpdateData();
	
	CDialog::OnOK();
}

void CUpdateLinksDlg::OnHelpUplinks() 
{
	theApp.WinHelp(IDD_UPDATE_LINKS + 0x20000, HELP_CONTEXT);	
}



