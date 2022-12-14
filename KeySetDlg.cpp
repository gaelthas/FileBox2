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

// KeySetDlg.cpp : implementation file
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
// CKeySetDlg dialog


CKeySetDlg::CKeySetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeySetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeySetDlg)
	m_nMaxRecent = 8;
	//}}AFX_DATA_INIT
}


void CKeySetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeySetDlg)
	DDX_Control(pDX, IDC_SYSMNU_TOPMOST, m_cSysTopmost);
	DDX_Control(pDX, IDC_SYSMNU_ROLLUP, m_cSysRollup);
	DDX_Control(pDX, IDC_RECENT_SORT, m_cRecentSort);
	DDX_Control(pDX, IDC_RECENT_PATH, m_cRecentPath);
	DDX_Control(pDX, IDC_ADDNUM_RECENT, m_cNumRec);
	DDX_Control(pDX, IDC_HOTKEY_REC, m_cHkRec);
	DDX_Control(pDX, IDC_HOTKEY_FAV, m_cHkFav);
	DDX_Text(pDX, IDC_MAX_RECENT, m_nMaxRecent);
	DDV_MinMaxInt(pDX, m_nMaxRecent, 0, 32);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_RECENT_FROMWIN, m_cRecentFromWin);
}


BEGIN_MESSAGE_MAP(CKeySetDlg, CDialog)
	//{{AFX_MSG_MAP(CKeySetDlg)
	ON_EN_KILLFOCUS(IDC_MAX_RECENT, OnKillfocusMaxRecent)
	ON_BN_CLICKED(IDC_RECENT_PATH, OnRecentPath)
	ON_BN_CLICKED(IDC_RECENT_SORT, OnRecentSort)
	ON_BN_CLICKED(IDC_SYSMNU_ROLLUP, OnSysmnuRollup)
	ON_BN_CLICKED(IDC_SYSMNU_TOPMOST, OnSysmnuTopmost)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RECENT_FROMWIN, &CKeySetDlg::OnBnClickedRecentFromwin)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeySetDlg message handlers

void CKeySetDlg::InitData() 
{
	if (pShData) {
		m_cHkFav.SetHotKey(pShData->wFavKey, pShData->wFavMod);
		m_cHkRec.SetHotKey(pShData->wRecKey, pShData->wRecMod);
		// m_cNumFav.SetCheck(pShData->bFavAddNum); // not used
		m_cNumRec.SetCheck(pShData->bRecAddNum);
		m_cRecentPath.SetCheck(pShData->bRecentPath);
		m_cRecentSort.SetCheck(pShData->bSortRecent);
		m_cSysTopmost.SetCheck(pShData->bSysTopmost);
		m_cSysRollup.SetCheck(pShData->bSysRollup);
		m_cRecentFromWin.SetCheck(pShData->bRecentFromWin);
		
		m_nMaxRecent = pShData->nMaxRecent;
		UpdateData(FALSE);
	}
}

BOOL CKeySetDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	pMainDlg->m_pKeyDlg = this;
	
	// For some reason Alt+key combinations do not work...
	m_cHkFav.SetRules(HKCOMB_NONE | HKCOMB_S | HKCOMB_A, HOTKEYF_SHIFT | HOTKEYF_CONTROL);
	m_cHkRec.SetRules(HKCOMB_NONE | HKCOMB_S | HKCOMB_A, HOTKEYF_SHIFT | HOTKEYF_CONTROL);
	InitData();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CKeySetDlg::SetKeys() 
{
	if (!pShData)
		return;
	m_cHkFav.GetHotKey(pShData->wFavKey, pShData->wFavMod);
	m_cHkRec.GetHotKey(pShData->wRecKey, pShData->wRecMod);
	// pShData->bFavAddNum = (bool) m_cNumFav.GetCheck(); // not used
	pShData->bRecAddNum = (bool) m_cNumRec.GetCheck();
}


void CKeySetDlg::OnKillfocusMaxRecent() 
{
	UpdateData();
	if (pShData) {
		pShData->nMaxRecent = m_nMaxRecent;
		if (pShData->nRecent > m_nMaxRecent)
			pShData->nRecent = m_nMaxRecent;
	}	
}


void CKeySetDlg::OnRecentPath() 
{
	if (pShData) 
		pShData->bRecentPath = (bool) m_cRecentPath.GetCheck();
}

void CKeySetDlg::OnRecentSort() 
{
	if (pShData) 
		pShData->bSortRecent = m_cRecentSort.GetCheck();	
}

void CKeySetDlg::OnSysmnuRollup() 
{
	if (pShData) 
		pShData->bSysRollup = m_cSysRollup.GetCheck();	
}

void CKeySetDlg::OnSysmnuTopmost() 
{
	if (pShData) 
		pShData->bSysTopmost = m_cSysTopmost.GetCheck();	
}

void CKeySetDlg::OnBnClickedRecentFromwin()
{
	if (pShData)
		pShData->bRecentFromWin = m_cRecentFromWin.GetCheck();
}
