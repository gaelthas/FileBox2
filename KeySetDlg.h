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

#if !defined(AFX_KEYSETDLG_H__EC9D76EF_0C6E_455E_814C_3A7181C2BDD5__INCLUDED_)
#define AFX_KEYSETDLG_H__EC9D76EF_0C6E_455E_814C_3A7181C2BDD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeySetDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeySetDlg dialog

class CKeySetDlg : public CDialog
{
// Construction
public:
	CKeySetDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeySetDlg)
	enum { IDD = IDD_KEY_SETTINGS };
	CButton	m_cSysTopmost;
	CButton	m_cSysRollup;
	CButton	m_cRecentSort;
	CButton	m_cRecentPath;
	CButton m_cRecentFromWin;
	CButton	m_cNumRec;
	CHotKeyCtrl	m_cHkRec;
	CHotKeyCtrl	m_cHkFav;
	int		m_nMaxRecent;
	//}}AFX_DATA

	void InitData();
	void SetKeys();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeySetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CKeySetDlg)
	virtual BOOL OnInitDialog();;
	afx_msg void OnKillfocusMaxRecent();
	afx_msg void OnRecentPath();
	afx_msg void OnRecentSort();
	afx_msg void OnSysmnuRollup();
	afx_msg void OnSysmnuTopmost();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRecentFromwin();
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYSETDLG_H__EC9D76EF_0C6E_455E_814C_3A7181C2BDD5__INCLUDED_)
