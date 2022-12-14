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

#if !defined(AFX_ITEMDLG_H__43DE0E2B_614B_11D3_85C3_0000C0597CC7__INCLUDED_)
#define AFX_ITEMDLG_H__43DE0E2B_614B_11D3_85C3_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ItemDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CItemDlg dialog

class CItemDlg : public CDialog
{
// Construction
public:
	CItemDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CItemDlg)
	enum { IDD = IDD_NEW_ITEM };
	CEdit	m_cItemTarget;
	CEdit	m_cItemDesc;
	CButton	m_cOK;
	CString	m_sDesc;
	CString	m_sTarget;
	CString	m_sItemType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CItemDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CItemDlg)
	afx_msg void OnBrowseFolder();
	afx_msg void OnBrowseFile();
	afx_msg void OnChangeItemDesc();
	afx_msg void OnChangeItemTarget();
	virtual BOOL OnInitDialog();
	afx_msg void OnHelpItprop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HWND m_hwndHelp;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ITEMDLG_H__43DE0E2B_614B_11D3_85C3_0000C0597CC7__INCLUDED_)
