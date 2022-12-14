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

#if !defined(AFX_SETTINGSDLG_H__0B5D7965_830E_11D3_8605_0000C0597CC7__INCLUDED_)
#define AFX_SETTINGSDLG_H__0B5D7965_830E_11D3_8605_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
// Construction
public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS };
	CButton	m_cFoldExpl;
	CButton	m_cTrayIcon;
	CButton	m_cRolledTopmost;
	CButton	m_cRollUp;
	CButton	m_cRecFB;
	CButton	m_cFavFB;
	CButton	m_cFavEx;
	CButton	m_cRecEx;
	CButton	m_cPushPin;
	int		m_nPixelsLeft;
	int		m_nMaxPixelsLeft;
	int		m_iResizeHeight;
	int		m_iResizeWidth;
	//}}AFX_DATA

	void InitData() ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSettingsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusMoveLeft();
	afx_msg void OnKillfocusMoveLeftmax();
	afx_msg void OnDispFavEx();
	afx_msg void OnDispFavFB();
	afx_msg void OnDispPushpin();
	afx_msg void OnDispRecEx();
	afx_msg void OnDispRecFB();
	afx_msg void OnDispRollUp();
	afx_msg void OnRolledTopmost();
	afx_msg void OnTrayIcon();
	afx_msg void OnFoldersExpl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeMoveLeft();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__0B5D7965_830E_11D3_8605_0000C0597CC7__INCLUDED_)
