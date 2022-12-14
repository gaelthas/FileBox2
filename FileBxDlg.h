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

#if !defined(AFX_FILEBXDLG_H__24EFC7C6_8244_11D3_8604_0000C0597CC7__INCLUDED_)
#define AFX_FILEBXDLG_H__24EFC7C6_8244_11D3_8604_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileBxDlg.h : header file
//
#include "HeaderDlg.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CFileBxDlg dialog

#define MAX_TAB_PAGES 6
#define LIC_PAGE 5

class CFileBxDlg : public CDialog
{
// Construction
public:
	void ResizeControls();
	CFileBxDlg(CWnd* pParent = NULL);   // standard constructor
	~CFileBxDlg();

// Dialog Data
	//{{AFX_DATA(CFileBxDlg)
	enum { IDD = IDD_FILEBX };
	CStatic	m_cHelpText;
	CTabCtrl	m_cTab;
	CButton	m_cHelp;
	CButton	m_cExit;
	CButton	m_cAbout;
	CButton	m_cClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileBxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON	m_hIcon;
	BOOL	m_bHookInstalled;
	HWND	m_hWndBar;

	// Generated message map functions
	//{{AFX_MSG(CFileBxDlg)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAbout();
	afx_msg void OnClose();
	afx_msg void OnHelpFbx();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnExitButton();
	afx_msg void OnExit();
	afx_msg void OnOpen();
	afx_msg void OnDestroy();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
public:
	afx_msg LRESULT OnTrayIconMsg(WPARAM wParam, LPARAM lParam);
protected:
	afx_msg void OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized );
	afx_msg void OnWindowPosChanging( WINDOWPOS* lpwndpos );
	afx_msg LRESULT OnNcHitTest( CPoint point );
	afx_msg BOOL OnQueryEndSession();
	afx_msg LRESULT OnTaskBarCreated(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnQuit(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

public:
	CSettingsDlg * m_pSetDlg;
	COptionsDlg * m_pOptsDlg;
	CFavoritesDlg * m_pFavDlg;
	CKeySetDlg * m_pKeyDlg;
	int m_nTabs;
	CDialog * m_pPage[MAX_TAB_PAGES];
	void SetHelpText(int iHelpId = IDS_PRESS_HELP);
	void SaveStatus();
	void ReadStatus();
	BOOL IsHookInstalled() { return m_bHookInstalled; }
	void ToggleHooks();
public:
	// Header dialog, may be NULL for a licensed version
	HeaderDlg *m_pHeaderDlg;
public:
	// Shows or hides info
	CButton m_cToggleInfo;
public:
	afx_msg void OnBnClickedToggleInfo();
public:
	// // Tab X position with info window shown
	int m_nTabPosX;
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
};

extern CFileBxDlg *pMainDlg;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEBXDLG_H__24EFC7C6_8244_11D3_8604_0000C0597CC7__INCLUDED_)
