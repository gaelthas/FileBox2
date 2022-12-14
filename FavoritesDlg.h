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

// DFolderDlg.h : header file
//

#include "DragTreeCtrl.h"

#if !defined(AFX_DFOLDERDLG_H__B20AAE08_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
#define AFX_DFOLDERDLG_H__B20AAE08_1F42_11D3_8563_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFavoritesDlg dialog

class CFavoritesDlg : public CDialog
{
// Construction
public:
	void ResizeControls();
	CFavoritesDlg(CWnd* pParent = NULL);	// standard constructor
	~CFavoritesDlg();

// Dialog Data
	//{{AFX_DATA(CFavoritesDlg)
	enum { IDD = IDD_FAVORITES };
	CButton	m_cDeleteAll;
	CButton	m_cDelete;
	CButton	m_cRename;
	CButton	m_cEditItem;
	CButton	m_cTest;
	CEdit	m_cItemInfo;
	CDragTreeCtrl	m_cTree;
	CEdit	m_cDbgMsg;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFavoritesDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFavoritesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTest();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddSeparator();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnNewSubmenu();
	afx_msg void OnNewShortcut();
	afx_msg void OnEditItem();
	afx_msg void OnDeleteAll();
	afx_msg void OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDestroy();
	afx_msg void OnKeydownTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateLinks();
	afx_msg void OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFolder();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
public:
	void AddFavorite(const TCHAR *pcDesc, const TCHAR *pcTarget, HTREEITEM hInsertAfter = NULL);
	BOOL DeleteFavorite(const TCHAR *szDesc);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DFOLDERDLG_H__B20AAE08_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
