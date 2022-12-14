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

#include "afxwin.h"
#if !defined(AFX_EXCEPTIONSDLG_H__7FA7D85F_5C6E_4631_9B42_5D1584891FBA__INCLUDED_)
#define AFX_EXCEPTIONSDLG_H__7FA7D85F_5C6E_4631_9B42_5D1584891FBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExceptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExceptionsDlg dialog

class CExceptionsDlg : public CDialog
{
// Construction
public:
	CExceptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExceptionsDlg)
	enum { IDD = IDD_EXCEPTIONS };
	CButton	m_cNoRoll;
	CButton	m_cVResize;
	CButton	m_cHResize;
	CButton	m_cFoldersBtn;
	CButton	m_cNoPin;
	CButton	m_cDontHandle;
	CButton	m_cDelExcBtn;
	CListBox	m_cExcList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExceptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExceptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeExcList();
	afx_msg void OnAddExcept();
	afx_msg void OnDelExcept();
	virtual void OnOK();
	afx_msg void OnExDonthandle();
	afx_msg void OnExNopin();
	afx_msg void OnExFolderbtns();
	afx_msg void OnExHresize();
	afx_msg void OnExVresize();
	afx_msg void OnExNoroll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_cMoveLeft;
	afx_msg void OnEnKillfocusMoveLeft();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXCEPTIONSDLG_H__7FA7D85F_5C6E_4631_9B42_5D1584891FBA__INCLUDED_)
