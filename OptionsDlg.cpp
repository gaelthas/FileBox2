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

// OptionsDlg.cpp : implementation file
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

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog


COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptionsDlg)
	m_nResizeHeight = 0;
	m_nResizeWidth = 0;
	m_nSortCol = -1;
	m_nSortOrder = -1;
	m_nClickSwitch = -1;
	//}}AFX_DATA_INIT
}


void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsDlg)
	DDX_Control(pDX, IDC_DETAILED_ASIZ, m_cDetailedASiz);
	DDX_Control(pDX, IDC_RESIZE, m_cResizeStdBox);
	DDX_Control(pDX, IDC_DETAILED, m_cDetailed);
	DDX_Text(pDX, IDC_RESIZE_HEIGHT, m_nResizeHeight);
	DDX_Text(pDX, IDC_RESIZE_WIDTH, m_nResizeWidth);
	DDX_Radio(pDX, IDC_SORT_COL1, m_nSortCol);
	DDX_Radio(pDX, IDC_SORT_ORD1, m_nSortOrder);
	DDX_Radio(pDX, IDC_CLICKSWITCH1, m_nClickSwitch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	//{{AFX_MSG_MAP(COptionsDlg)
	ON_BN_CLICKED(IDC_DETAILED, OnDetailed)
	ON_BN_CLICKED(IDC_RESIZE, OnResize)
	ON_EN_KILLFOCUS(IDC_RESIZE_WIDTH, OnKillfocusResizeWidth)
	ON_EN_KILLFOCUS(IDC_RESIZE_HEIGHT, OnKillfocusResizeHeight)
	ON_BN_CLICKED(IDC_DETAILED_ASIZ, OnDetailedAsiz)
	ON_BN_CLICKED(IDC_SORT_COL1, OnSortCol1)
	ON_BN_CLICKED(IDC_SORT_COL2, OnSortCol2)
	ON_BN_CLICKED(IDC_SORT_COL3, OnSortCol3)
	ON_BN_CLICKED(IDC_SORT_COL4, OnSortCol4)
	ON_BN_CLICKED(IDC_SORT_ORD1, OnSortOrd1)
	ON_BN_CLICKED(IDC_SORT_ORD2, OnSortOrd2)
	ON_BN_CLICKED(IDC_CLICKSWITCH1, OnClickswitch1)
	ON_BN_CLICKED(IDC_CLICKSWITCH2, OnClickswitch2)
	ON_BN_CLICKED(IDC_CLICKSWITCH3, OnClickswitch3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg message handlers

void COptionsDlg::EnableControls()
{
	BOOL bEnable = m_cDetailed.GetCheck();
	m_cDetailedASiz.EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_COL1)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_COL2)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_COL3)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_COL4)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_ORD1)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_ORD2)->EnableWindow(bEnable);
	GetDlgItem(IDC_SORT_FRAME)->EnableWindow(bEnable);
}

void COptionsDlg::InitData() 
{
	if (pShData) {
		m_cDetailed.SetCheck(pShData->dwDetailsView & 1);
		m_cDetailedASiz.SetCheck(pShData->dwDetailsView & 2);
		m_cResizeStdBox.SetCheck(pShData->nResizeFlags & 1);
		m_nResizeHeight = pShData->nResizeHeight;
		m_nResizeWidth = pShData->nResizeWidth;
		m_nSortCol = (pShData->dwDetailsView & 0xF0) >> 4;
		m_nSortOrder = (pShData->dwDetailsView & 4) ? 1 : 0;
		m_nClickSwitch = pShData->nWantClickDir;
	}
	EnableControls();
	UpdateData(FALSE);
}

BOOL COptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	pMainDlg->m_pOptsDlg = this;
	InitData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsDlg::OnDetailed() 
{
	if (pShData) {
		if (m_cDetailed.GetCheck())
			pShData->dwDetailsView |= 1;
		else
			pShData->dwDetailsView &= ~1;
	}
	EnableControls();
}

void COptionsDlg::OnDetailedAsiz() 
{
	if (pShData) {
		if (m_cDetailedASiz.GetCheck())
			pShData->dwDetailsView |= 2;
		else
			pShData->dwDetailsView &= ~2;
	}
}

void COptionsDlg::OnResize() 
{
	if (pShData) {
		if (m_cResizeStdBox.GetCheck())
			pShData->nResizeFlags |= 1;
		else
			pShData->nResizeFlags &= ~1;
	}
}

void COptionsDlg::OnKillfocusResizeWidth() 
{
	UpdateData();
	if (m_nResizeWidth < 75)
		m_nResizeWidth = 75;
	else if (m_nResizeWidth > 300)
		m_nResizeWidth = 300;
	UpdateData(FALSE);

	if (pShData) {
		pShData->nResizeWidth = m_nResizeWidth;
	}	
}

void COptionsDlg::OnKillfocusResizeHeight() 
{
	UpdateData();
	if (m_nResizeHeight < 75)
		m_nResizeHeight = 75;
	else if (m_nResizeHeight > 300)
		m_nResizeHeight = 300;
	UpdateData(FALSE);

	if (pShData) {
		pShData->nResizeHeight = m_nResizeHeight;
	}	
}


void COptionsDlg::OnSortCol1() 
{
	if (pShData) {
		pShData->dwDetailsView &= 0xFFFFFF0F; // sort by name
	}
}

void COptionsDlg::OnSortCol2() 
{
	if (pShData) {
		pShData->dwDetailsView &= 0xFFFFFF0F; // sort by size
		pShData->dwDetailsView |= 0x10; // sort by name
	}
}

void COptionsDlg::OnSortCol3() 
{
	if (pShData) {
		pShData->dwDetailsView &= 0xFFFFFF0F; // sort by type
		pShData->dwDetailsView |= 0x20; // sort by name
	}
}

void COptionsDlg::OnSortCol4() 
{
	if (pShData) {
		pShData->dwDetailsView &= 0xFFFFFF0F; // sort by date
		pShData->dwDetailsView |= 0x30; // sort by name
	}
}

void COptionsDlg::OnSortOrd1() 
{
	if (pShData) {
		pShData->dwDetailsView &= ~0x4; // sort ascending
	}
}

void COptionsDlg::OnSortOrd2() 
{
	if (pShData) {
		pShData->dwDetailsView |= 0x4; // sort descending
	}
}


void COptionsDlg::OnClickswitch1() 
{
	UpdateData();
	pShData->nWantClickDir = m_nClickSwitch;
}

void COptionsDlg::OnClickswitch2() 
{
	UpdateData();
	pShData->nWantClickDir = m_nClickSwitch;
}

void COptionsDlg::OnClickswitch3() 
{
	UpdateData();
	pShData->nWantClickDir = m_nClickSwitch;
}
