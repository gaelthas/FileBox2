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

// SettingsDlg.cpp : implementation file
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
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDlg)
	m_nPixelsLeft = 0;
	m_nMaxPixelsLeft = 0;
	m_iResizeHeight = 0;
	m_iResizeWidth = 0;
	//}}AFX_DATA_INIT
}


void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Control(pDX, IDC_FOLDERS_EXPL, m_cFoldExpl);
	DDX_Control(pDX, IDC_TRAY_ICON, m_cTrayIcon);
	DDX_Control(pDX, IDC_ROLLED_TOPMOST, m_cRolledTopmost);
	DDX_Control(pDX, IDC_DISP_ROLLUP, m_cRollUp);
	DDX_Control(pDX, IDC_DISP_RECFB, m_cRecFB);
	DDX_Control(pDX, IDC_DISP_FAVFB, m_cFavFB);
	DDX_Control(pDX, IDC_DISP_FAVEX, m_cFavEx);
	DDX_Control(pDX, IDC_DISP_RECEX, m_cRecEx);
	DDX_Control(pDX, IDC_DISP_PUSHPIN, m_cPushPin);
	DDX_Text(pDX, IDC_MOVE_LEFT, m_nPixelsLeft);
	DDV_MinMaxInt(pDX, m_nPixelsLeft, -600, 600);
	DDX_Text(pDX, IDC_MOVE_LEFTMAX, m_nMaxPixelsLeft);
	DDV_MinMaxInt(pDX, m_nMaxPixelsLeft, -1000, 1000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_EN_KILLFOCUS(IDC_MOVE_LEFT, OnKillfocusMoveLeft)
	ON_EN_KILLFOCUS(IDC_MOVE_LEFTMAX, OnKillfocusMoveLeftmax)
	ON_BN_CLICKED(IDC_DISP_FAVEX, OnDispFavEx)
	ON_BN_CLICKED(IDC_DISP_FAVFB, OnDispFavFB)
	ON_BN_CLICKED(IDC_DISP_PUSHPIN, OnDispPushpin)
	ON_BN_CLICKED(IDC_DISP_RECEX, OnDispRecEx)
	ON_BN_CLICKED(IDC_DISP_RECFB, OnDispRecFB)
	ON_BN_CLICKED(IDC_DISP_ROLLUP, OnDispRollUp)
	ON_BN_CLICKED(IDC_ROLLED_TOPMOST, OnRolledTopmost)
	ON_BN_CLICKED(IDC_TRAY_ICON, OnTrayIcon)
	ON_BN_CLICKED(IDC_FOLDERS_EXPL, OnFoldersExpl)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_MOVE_LEFT, &CSettingsDlg::OnEnChangeMoveLeft)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

void CSettingsDlg::InitData() 
{
	if (pShData)
	{
		m_nPixelsLeft = pShData->nPixelsLeft;
		m_nMaxPixelsLeft = pShData->nMaxPixelsLeft;
		m_cPushPin.SetCheck(pShData->dwButtons & BTN_PUSHPIN);
		m_cRollUp.SetCheck(pShData->dwButtons & BTN_ROLLUP);
		m_cRolledTopmost.SetCheck(pShData->bRolledTopmost);
		m_cRolledTopmost.EnableWindow(m_cRollUp.GetCheck());
		m_cFoldExpl.SetCheck(pShData->dwButtons & FLD_EXPLORE);
		m_cFavEx.SetCheck(pShData->dwButtons & BTN_FAV_EX);
		m_cFavFB.SetCheck(pShData->dwButtons & BTN_FAV_FB);
		m_cRecEx.SetCheck(pShData->dwButtons & BTN_REC_EX);
		m_cRecFB.SetCheck(pShData->dwButtons & BTN_REC_FB);
		m_cTrayIcon.EnableWindow(pShData->dwButtons);
	}
	UpdateData(FALSE);
}

BOOL CSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	pMainDlg->m_pSetDlg = this;
	InitData();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDlg::OnKillfocusMoveLeft() 
{
	UpdateData();
	if (pShData) {
		pShData->nPixelsLeft = m_nPixelsLeft;
	}	
}

void CSettingsDlg::OnKillfocusMoveLeftmax() 
{
	UpdateData();
	if (pShData) {
		pShData->nMaxPixelsLeft = m_nMaxPixelsLeft;
	}	
}

void CSettingsDlg::OnDispPushpin() 
{
	if (m_cPushPin.GetCheck())
		pShData->dwButtons |= BTN_PUSHPIN;
	else {
		pShData->dwButtons &= ~BTN_PUSHPIN;
	}
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}

void CSettingsDlg::OnDispRollUp() 
{
	if (m_cRollUp.GetCheck())
		pShData->dwButtons |= BTN_ROLLUP;
	else {
		pShData->dwButtons &= ~BTN_ROLLUP;
		// Unroll windows
		for (int i = 0; i < MAX_HWDATA; i++) {
			if (pShData->hw[i].hwnd) {
				HW_DATA *phd = &pShData->hw[i];
				if (IsWindow(phd->hwnd) && phd->dwOldHeight) {
					::SendMessage(phd->hwnd, WM_NULL, WP_UNROLL_MSG, LP_UNROLL_MSG);
				}
			}
		}
	}
	m_cRolledTopmost.EnableWindow(m_cRollUp.GetCheck());
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}


void CSettingsDlg::OnDispFavEx() 
{
	if (m_cFavEx.GetCheck())
		pShData->dwButtons |= BTN_FAV_EX;
	else
		pShData->dwButtons &= ~BTN_FAV_EX;
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}

void CSettingsDlg::OnDispFavFB() 
{
	if (m_cFavFB.GetCheck())
		pShData->dwButtons |= BTN_FAV_FB;
	else
		pShData->dwButtons &= ~BTN_FAV_FB;
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}

void CSettingsDlg::OnDispRecEx() 
{
	if (m_cRecEx.GetCheck())
		pShData->dwButtons |= BTN_REC_EX;
	else
		pShData->dwButtons &= ~BTN_REC_EX;
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}

void CSettingsDlg::OnDispRecFB() 
{
	if (m_cRecFB.GetCheck())
		pShData->dwButtons |= BTN_REC_FB;
	else
		pShData->dwButtons &= ~BTN_REC_FB;
	if (pMainDlg->IsHookInstalled() != (BOOL) ((pShData->dwButtons  & 0x7fff) != 0))
		pMainDlg->ToggleHooks();
	pMainDlg->OnTrayIconMsg(0, 0);
}

void CSettingsDlg::OnRolledTopmost() 
{
	pShData->bRolledTopmost = (bool) m_cRolledTopmost.GetCheck();
}



void CSettingsDlg::OnTrayIcon() 
{
	if (m_cTrayIcon.GetCheck())
		pMainDlg->OnTrayIconMsg(0, 0); // Set tray icon;
	else
		pMainDlg->OnTrayIconMsg(1, 0); // Remove tray icon		
}

void CSettingsDlg::OnFoldersExpl() 
{
	if (pShData) {
		if (m_cFoldExpl.GetCheck())
			pShData->dwButtons |= FLD_EXPLORE;
		else
			pShData->dwButtons &= ~FLD_EXPLORE;
	}
}

void CSettingsDlg::OnEnChangeMoveLeft()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
