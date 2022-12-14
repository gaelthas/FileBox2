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

#if !defined(AFX_OPTIONSDLG_H__99CF4991_13E8_4073_AD64_3EA2D9762596__INCLUDED_)
#define AFX_OPTIONSDLG_H__99CF4991_13E8_4073_AD64_3EA2D9762596__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

class COptionsDlg : public CDialog
{
// Construction
public:
	void EnableControls();
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_OPTIONS };
	CButton	m_cDetailedASiz;
	CButton	m_cResizeStdBox;
	CButton	m_cDetailed;
	short	m_nResizeHeight;
	short	m_nResizeWidth;
	int		m_nSortCol;
	int		m_nSortOrder;
	int		m_nClickSwitch;
	//}}AFX_DATA

	void InitData();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDetailed();
	afx_msg void OnResize();
	afx_msg void OnKillfocusResizeWidth();
	afx_msg void OnKillfocusResizeHeight();
	afx_msg void OnDetailedAsiz();
	afx_msg void OnSortCol1();
	afx_msg void OnSortCol2();
	afx_msg void OnSortCol3();
	afx_msg void OnSortCol4();
	afx_msg void OnSortOrd1();
	afx_msg void OnSortOrd2();
	afx_msg void OnClickswitch1();
	afx_msg void OnClickswitch2();
	afx_msg void OnClickswitch3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDLG_H__99CF4991_13E8_4073_AD64_3EA2D9762596__INCLUDED_)
