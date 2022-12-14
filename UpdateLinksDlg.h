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

#if !defined(AFX_UPDATELINKSDLG_H__2754C665_8D4F_11D3_8618_0000C0597CC7__INCLUDED_)
#define AFX_UPDATELINKSDLG_H__2754C665_8D4F_11D3_8618_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdateLinksDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdateLinksDlg dialog

class CUpdateLinksDlg : public CDialog
{
// Construction
public:
	CUpdateLinksDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUpdateLinksDlg)
	enum { IDD = IDD_UPDATE_LINKS };
	int		m_iUpdateMethod;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateLinksDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdateLinksDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnHelpUplinks();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnBnClickedRadio1();
public:
//	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATELINKSDLG_H__2754C665_8D4F_11D3_8618_0000C0597CC7__INCLUDED_)
