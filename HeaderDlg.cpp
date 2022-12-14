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

// HeaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "resource.h"
#include "HeaderDlg.h"


// HeaderDlg dialog

IMPLEMENT_DYNCREATE(HeaderDlg, CDHtmlDialog)

HeaderDlg::HeaderDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(HeaderDlg::IDD, HeaderDlg::IDH, pParent)
{

}

HeaderDlg::~HeaderDlg()
{
}

void HeaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL HeaderDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	CString sInfo = theApp.m_sProgDir + "info\\info.htm";
	if (_access(sInfo, 0) == 0)
		Navigate("file://" + sInfo);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(HeaderDlg, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(HeaderDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()



// HeaderDlg message handlers

HRESULT HeaderDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK;
}

HRESULT HeaderDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK;
}
