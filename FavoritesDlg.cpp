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

// DFolderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "OptionsDlg.h"
#include "KeySetDlg.h"
#include "FileBxDlg.h"
#include "ItemDlg.h"
#include "UpdateLinksDlg.h"
#include "DFH/DFH_shared.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString g_sCfgDir;

/////////////////////////////////////////////////////////////////////////////
// CFavoritesDlg dialog

CFavoritesDlg::CFavoritesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFavoritesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFavoritesDlg)
	//}}AFX_DATA_INIT
}

CFavoritesDlg::~CFavoritesDlg()
{
}

void CFavoritesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFavoritesDlg)
	DDX_Control(pDX, IDC_DELETE_ALL, m_cDeleteAll);
	DDX_Control(pDX, IDC_DELETE, m_cDelete);
	DDX_Control(pDX, IDC_RENAME, m_cRename);
	DDX_Control(pDX, IDC_EDIT_ITEM, m_cEditItem);
	DDX_Control(pDX, IDC_TEST, m_cTest);
	DDX_Control(pDX, IDC_ITEM_INFO, m_cItemInfo);
	DDX_Control(pDX, IDC_TREE1, m_cTree);
	DDX_Control(pDX, IDC_DBG_MSG, m_cDbgMsg);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFavoritesDlg, CDialog)
	//{{AFX_MSG_MAP(CFavoritesDlg)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_TEST, OnTest)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnSelchangedTree1)
	ON_BN_CLICKED(IDC_ADD_SEPARATOR, OnAddSeparator)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_NEW_SUBMENU, OnNewSubmenu)
	ON_BN_CLICKED(IDC_NEW_SHORTCUT, OnNewShortcut)
	ON_BN_CLICKED(IDC_EDIT_ITEM, OnEditItem)
	ON_BN_CLICKED(IDC_DELETE_ALL, OnDeleteAll)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE1, OnEndlabeleditTree1)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE1, OnBeginlabeleditTree1)
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_KEYDOWN, IDC_TREE1, OnKeydownTree1)
	ON_BN_CLICKED(IDC_UPDATE_LINKS, OnUpdateLinks)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnDblclkTree1)
	ON_BN_CLICKED(IDC_CHANGE_FOLDER, OnChangeFolder)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFavoritesDlg message handlers

BOOL CFavoritesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	pMainDlg->m_pFavDlg = this;

	// This is to avoid explicit loading on startup of SHFolder.dll
	typedef HRESULT (CALLBACK *SHGetFolderPathType)(
		HWND hwndOwner,
		int nFolder,
		HANDLE hToken,
		DWORD dwFlags,
		LPTSTR pszPath
	);
	SHGetFolderPathType SHGetFolderPath = NULL;
	HMODULE hDLL = LoadLibrary(_T("shell32.dll"));
	if (hDLL)
		SHGetFolderPath = (SHGetFolderPathType) GetProcAddress(hDLL, "SHGetFolderPathA"); 
	if (!SHGetFolderPath) {
		if (hDLL)
			::FreeLibrary(hDLL);
		hDLL = ::LoadLibrary(_T("shfolder.dll"));
		if (hDLL)
			SHGetFolderPath = (SHGetFolderPathType) GetProcAddress(hDLL, "SHGetFolderPathA");
	}

	if (g_sCfgDir == _T(""))
		g_sCfgDir = theApp.RegGetString(_T("Settings"), _T("CfgDir"), _T(""));

	if (g_sCfgDir > _T("") && _access(g_sCfgDir, 0) == 0) 
	{
		strcpy(m_cTree.m_pszFolder, g_sCfgDir);
	}
	else
	{
		TCHAR buf[MAX_PATH];
		*buf = 0;
		
		BOOL bMakeNew = TRUE;

		if (SHGetFolderPath) {
			if (S_OK == SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, buf)) {
				TCHAR *pc = buf + strlen(buf) - 2;
				while (pc > buf && !(*pc == '\\' || *pc == '/'))
					pc--;
				if (*pc == '\\' || *pc == '/')
					*pc = 0;
				strcat(buf, _T("\\Application Data\\Hyperionics\\fbx"));
				if (_access(buf, 0) == 0)
					bMakeNew = FALSE;
			}

			if (bMakeNew) {
				*buf = 0;
				if (E_INVALIDARG != SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buf) && *buf) 	{
					CreateDirectory(buf, NULL);

				} else if (S_OK == SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, buf)) 	{
					TCHAR *pc = buf + strlen(buf) - 2;
					while (pc > buf && !(*pc == '\\' || *pc == '/'))
						pc--;
					if (*pc == '\\' || *pc == '/') {
						*pc = 0;
						strcat(buf, _T("\\Application Data"));
						CreateDirectory(buf, NULL);
					}
				}
			}
		}
		if (hDLL)
			::FreeLibrary(hDLL);
		if (bMakeNew) {
			if (!buf[0]) {
				GetCurrentDirectory(MAX_PATH, buf);
			}
			strcat(buf, _T("\\Hyperionics"));
			CreateDirectory(buf, NULL);
			strcat(buf, _T("\\fbx"));
		}

		strcpy(m_cTree.m_pszFolder, buf);
		if (CreateDirectory(buf, NULL)) {
			theApp.m_bAddToUninstall = TRUE;
		} else if (GetLastError() != ERROR_ALREADY_EXISTS) {
			Msg(_T("CreateDirectory() error %d"), GetLastError());
		}
	}


	pMainDlg->ReadStatus();

	CImageList *pImList = new CImageList;
	int siz = 12;
	pImList->Create(siz, siz, ILC_COLOR | ILC_MASK, 8, 8);

	HICON hIcon;

	hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FAVORITES), 
		IMAGE_ICON, siz, siz, LR_DEFAULTCOLOR);
	if (hIcon)
		pImList->Add(hIcon);
	
	hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FILE), 
		IMAGE_ICON, siz, siz, LR_DEFAULTCOLOR);
	if (hIcon)
		pImList->Add(hIcon);

	hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_FOLDER), 
		IMAGE_ICON, siz, siz, LR_DEFAULTCOLOR);
	if (hIcon)
		pImList->Add(hIcon);

	hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_TEXT), 
		IMAGE_ICON, siz, siz, LR_DEFAULTCOLOR);
	if (hIcon)
		pImList->Add(hIcon);

	hIcon = (HICON) LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_SEPARATOR), 
		IMAGE_ICON, siz, siz, LR_DEFAULTCOLOR);
	if (hIcon)
		pImList->Add(hIcon);

	pImList->SetBkColor(m_cTree.GetBkColor());

	m_cTree.SetImageList(pImList, TVSIL_NORMAL);
	m_cTree.RefreshTree();

	ResizeControls();
	UpdateData(FALSE);

	HTREEITEM hItem = m_cTree.GetFirstVisibleItem();
	int m, n = -1;
	if (hItem) {
		m_cTree.SelectItem(hItem);
		m_cTree.GetItemImage(hItem, n, m);
	}
	BOOL bEnable = FALSE;
	if (m_cTree.GetSelectedItem())
		bEnable = TRUE;

	m_cEditItem.EnableWindow(bEnable && n != SEP_IMAGE);
	m_cRename.EnableWindow(bEnable && n != SEP_IMAGE);
	m_cDelete.EnableWindow(bEnable);
	m_cDeleteAll.EnableWindow(hItem != NULL);

	//m_cTree.SetFocus();
	if (pShData)
		m_cTree.CreateMenuResource((BYTE *) pShData->pcFavorites, sizeof(pShData->pcFavorites));

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BOOL CFavoritesDlg::DestroyWindow() 
{
	m_cTree.EnableFolderWatch(FALSE);
	if (m_cTree.GetImageList(TVSIL_NORMAL))
		delete m_cTree.GetImageList(TVSIL_NORMAL);
	return CDialog::DestroyWindow();
}


void CFavoritesDlg::ResizeControls()
{
#define BOTTOM_MARG	4
	if (!m_cTree.m_hWnd || !IsWindow(m_cTree.m_hWnd))
		return;
	CRect rct, rTree, r, rr;

	GetClientRect(&rct);

#ifdef _DEBUG
	int iDbgHeight;
	m_cDbgMsg.GetWindowRect(&r);
	ScreenToClient(&r);
	iDbgHeight = r.Height();
	r.top = rct.Height() - iDbgHeight - BOTTOM_MARG + 2;
	r.bottom = r.top + iDbgHeight;
	r.right = rct.Width() - 4;
	m_cDbgMsg.ShowWindow(SW_SHOW);

	m_cDbgMsg.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), 
		SWP_DRAWFRAME | SWP_NOZORDER);
#else
	int iDbgHeight = 0;
#endif

	m_cTree.GetWindowRect(&rTree);
	ScreenToClient(&rTree);
	rTree.right = rct.Width() - 4;
	rTree.bottom = rct.Height() - iDbgHeight - BOTTOM_MARG;

	m_cTree.SetWindowPos(NULL, rTree.left, rTree.top, rTree.Width(), rTree.Height(), 
		SWP_DRAWFRAME | SWP_NOZORDER);

	m_cItemInfo.GetWindowRect(&r);
	ScreenToClient(&r);
	r.bottom = rct.Height() - BOTTOM_MARG;
	m_cItemInfo.SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), 
		SWP_DRAWFRAME | SWP_NOZORDER);

}

void CFavoritesDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (nType == SIZE_MINIMIZED) {
		OnClose();
		return;
	}
	ResizeControls();	
}

BOOL CFavoritesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (wParam == IDCANCEL) {
		if (GetFocus())
			GetFocus()->SendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
		return FALSE;
	} else if (wParam == IDOK) {
		if (GetFocus())
			GetFocus()->SendMessage(WM_KEYDOWN, VK_RETURN, 0);
		return FALSE;
	}

	return CDialog::OnCommand(wParam, lParam);
}


void CFavoritesDlg::OnTest() 
{
	bool bDelete = FALSE;
	if (!pShData) {
		pShData = new DF_SHARED_DATA;
		memset(pShData, 0, sizeof(DF_SHARED_DATA));
		bDelete = TRUE;
	}
	BYTE *pMenuBuffer = (BYTE *) pShData->pcFavorites;
	if (m_cTree.CreateMenuResource(pMenuBuffer, sizeof(pShData->pcFavorites))) {
		HMENU hMenu = LoadMenuIndirect((MENUTEMPLATE *) pMenuBuffer);
		if (hMenu) {
			int iRet = 0;
			HMENU hSup = ::CreateMenu();
			::AppendMenu(hSup, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, _T("R"));
			CRect rct;
			m_cTest.GetWindowRect(&rct);
			iRet = ::TrackPopupMenu(hMenu, TPM_RETURNCMD, rct.left, rct.bottom, 0, m_hWnd, NULL);
			DestroyMenu(hSup);
			Msg(_T("iRet = %d"), iRet);
			if (iRet >= 10) {
				TCHAR *pcDir = pShData->pcFavorites + pShData->dwFavDescOffset + iRet;
				BOOL bFolder = pcDir[strlen(pcDir) - 1] == '\\';
				TCHAR *pcPar = NULL;
				if (!bFolder && *pcDir == '\"') {
					pcPar = strchr(pcDir + 1, '\"');
					if (pcPar) {
						pcPar++;
						*pcPar = 0;
						pcPar++;
					}
				}
				bFolder = bFolder && (pShData->dwButtons & FLD_EXPLORE);
				ShellExecute(m_hWnd, bFolder ? _T("explore") : _T("open"), pcDir, pcPar, _T(".\\"), SW_SHOW);
			}
		}
	}
	if (bDelete) {
		delete pShData;
		pShData = NULL;
	}
	pMainDlg->SetHelpText();
}

void CFavoritesDlg::OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	TCHAR buf[1024];

	int nlines = m_cItemInfo.GetLineCount();
	int npos = m_cItemInfo.LineIndex(nlines+1);
	m_cItemInfo.SetSel(0, npos);
	*buf = 0;

	unsigned u = 0;

	WIN32_FIND_DATA wfd;
	FILE *fp;
	TCHAR cPath[MAX_PATH + 20];
	HTREEITEM hItem = pNMTreeView ? pNMTreeView->itemNew.hItem : m_cTree.GetSelectedItem();

	if (hItem) {
		CString s1, s2;
		int n, m;
		m_cTree.GetItemImage(hItem, n, m);
		switch (n) {
		case FAV_FOLDER_IMAGE:
		case DOC_IMAGE:
			s1.LoadString(n == FAV_FOLDER_IMAGE ? IDS_FOLDER_SHORTCUT : IDS_FILE_SHORTCUT);
			s2 = m_cTree.GetFileName(hItem);
			if (ResolveShortCut(m_hWnd, s2, wfd, cPath) == 0)
				s2 = cPath;
			else 
				s2.LoadString(IDS_BROKEN_SHORTCUT);
			break;

		case FOLDER_IMAGE:
			s1.LoadString(IDS_IS_SUBMENU);
			break;

		case TEXT_IMAGE:
			s1.LoadString(IDS_IS_TEXT);
			s2 = m_cTree.GetFileName(hItem);
			if (fp = fopen(s2, _T("r"))) {
				*buf = 0;
				fgets(buf, MAX_PATH+20, fp);
				fclose(fp);
				TCHAR *pc = buf + strlen(buf);
				while (pc > buf && *pc < ' ')
					*pc-- = 0;
				s2 = buf;
			}
			break;

		case SEP_IMAGE:
			s1.LoadString(IDS_IS_SEPARATOR);
			break;
		}

		sprintf(buf, _T("%s\r\n%s\r\n%s"),
			m_cTree.GetItemText(hItem),
			s1,
			s2
		);
		m_cEditItem.EnableWindow(TRUE && n != SEP_IMAGE && n != FOLDER_IMAGE);
		m_cRename.EnableWindow(TRUE && n != SEP_IMAGE);
		m_cDelete.EnableWindow(TRUE);
		m_cDeleteAll.EnableWindow(m_cTree.GetFirstVisibleItem() != NULL);

	} else {
		m_cEditItem.EnableWindow(FALSE);
		m_cRename.EnableWindow(FALSE);
		m_cDelete.EnableWindow(FALSE);
		m_cDeleteAll.EnableWindow(m_cTree.GetFirstVisibleItem() != NULL);
	}
	m_cItemInfo.ReplaceSel(buf);

	pMainDlg->SetHelpText();	
	if (pResult)
		*pResult = 0;
}


void CFavoritesDlg::OnAddSeparator() 
{
	HTREEITEM hParentItem = TVI_ROOT;
	HTREEITEM hInsertAfter = m_cTree.GetSelectedItem();
	if (hInsertAfter)
		hParentItem = m_cTree.GetParentItem(hInsertAfter);
	else
		hInsertAfter = TVI_LAST;

	TCHAR szPath[MAX_PATH + 20], buf[MAX_PATH + 20];
	m_cTree.GetFileName(hInsertAfter, szPath);
	int n;
	if (hInsertAfter != TVI_LAST)
		n = (int) m_cTree.GetItemData(hInsertAfter);
	else
		n = 0xf0;
	FILE *fp = NULL;
	m_cTree.EnableFolderWatch(FALSE);
	do {
		n++;
		sprintf(buf, _T("%s\\---.FX%02x"), szPath, n);
		if (_access(buf, 0) < 0)
			fp = fopen(buf, _T("w"));
	} while (!fp && n < 0xff);
	if (!fp)
		return;
	fprintf(fp, _T("---"));
	fclose(fp);
	HTREEITEM hItem = m_cTree.InsertItem(_T("---"), SEP_IMAGE, SEP_IMAGE, hParentItem, hInsertAfter);
	m_cTree.SetItemData(hItem, n);
	m_cTree.RenumberItems(hParentItem);
	m_cTree.EnableFolderWatch(TRUE);
	m_cTree.SelectItem(hItem);
	pMainDlg->SetHelpText(IDS_MOVE_ITEM);
}

void CFavoritesDlg::OnDelete() 
{
	HTREEITEM hItem = m_cTree.GetSelectedItem();
	if (!hItem)
		return;
	CString s = m_cTree.GetFileName(hItem);
	m_cTree.EnableFolderWatch(FALSE);
	if (MoveFileOrFolder(s, NULL)) {
		m_cTree.DeleteItem(hItem);
		pMainDlg->SetHelpText(IDS_DELETE_HINT);
	}
	m_cTree.EnableFolderWatch(TRUE);
	if (!m_cTree.GetSelectedItem())
		OnSelchangedTree1(NULL, NULL);
}

void CFavoritesDlg::OnRename() 
{
	HTREEITEM hItem = m_cTree.GetSelectedItem();
	if (!hItem)
		return;
	m_cTree.EditLabel(hItem);	
}

void CFavoritesDlg::OnBeginlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	HTREEITEM hItem = pTVDispInfo->item.hItem;
	int n, m;
	m_cTree.GetItemImage(hItem, n, m);
	if (n != SEP_IMAGE)
		pMainDlg->SetHelpText(IDS_TYPE_NEW_NAME);
	*pResult = (n == SEP_IMAGE);
}


void CFavoritesDlg::OnEndlabeleditTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	if (pTVDispInfo->item.pszText) {
		if (!m_cTree.RenameItem(pTVDispInfo->item.hItem, pTVDispInfo->item.pszText))
			AfxMessageBox(IDS_ERR_RENAME, MB_OK | MB_ICONSTOP);
	}
	pMainDlg->SetHelpText();	
	*pResult = 0;
}


void CFavoritesDlg::OnNewSubmenu() 
{
	HTREEITEM hParentItem = TVI_ROOT;
	HTREEITEM hInsertAfter = m_cTree.GetSelectedItem();
	if (hInsertAfter)
		hParentItem = m_cTree.GetParentItem(hInsertAfter);
	else
		hInsertAfter = TVI_LAST;

	TCHAR szPath[MAX_PATH + 20], buf[MAX_PATH + 20];
	m_cTree.GetFileName(hInsertAfter, szPath);
	int n;
	if (hInsertAfter != TVI_LAST)
		n = (int) m_cTree.GetItemData(hInsertAfter);
	else
		n = 0xf0;
	BOOL bRet;
	CString sNewSub;
	sNewSub.LoadString(IDS_NEW_SUBMENU);
	m_cTree.EnableFolderWatch(FALSE);
	do {
		n++;
		sprintf(buf, _T("%s\\%s.FX%02x"), szPath, sNewSub, n);
		bRet = CreateDirectory(buf, NULL);
	} while (!bRet && n < 0xff);
	if (!bRet)
		return;
	HTREEITEM hItem = m_cTree.InsertItem(sNewSub, FOLDER_IMAGE, FOLDER_IMAGE, hParentItem, hInsertAfter);
	m_cTree.SetItemData(hItem, n);
	m_cTree.SelectItem(hItem);
	m_cTree.RenumberItems(hParentItem);
	m_cTree.EnableFolderWatch(TRUE);
	pMainDlg->SetHelpText(IDS_MOVE_ITEM);
	m_cTree.EditLabel(hItem);
}


void CFavoritesDlg::OnNewShortcut() 
{
	CItemDlg dlg(this);
	if (dlg.DoModal() == IDOK) {
		AddFavorite(dlg.m_sDesc, dlg.m_sTarget, m_cTree.GetSelectedItem());
		pMainDlg->SetHelpText(IDS_MOVE_ITEM);
	} else
		pMainDlg->SetHelpText(IDS_CREATE_DROP);
}


void CFavoritesDlg::OnEditItem() 
{
	HTREEITEM hItem = m_cTree.GetSelectedItem();
	if (!hItem)
		return;
	int n = -1;
	m_cTree.GetItemImage(hItem, n, n);
	if (n != FAV_FOLDER_IMAGE && n != DOC_IMAGE && n != TEXT_IMAGE)
		return;

	CItemDlg dlg(this);
	TCHAR buf[MAX_PATH + 20], *pc;
	*buf = 0;
	dlg.m_sDesc = m_cTree.GetItemText(hItem);
	CString sFile = m_cTree.GetFileName(hItem);
	if (n == TEXT_IMAGE) {
		FILE *fp;
		fp = fopen(sFile, _T("r"));
		if (fp) {
			fgets(buf, MAX_PATH, fp);
			fclose(fp);
			pc = buf + strlen(buf) - 1;
			while (pc > buf && *pc < ' ')
				*pc-- = 0;
		}
	} else {
		WIN32_FIND_DATA wfd;
		ResolveShortCut(m_hWnd, sFile, wfd, buf);
	}

	dlg.m_sTarget = buf;

	if (dlg.DoModal() == IDOK) {
		int nImage;

		if (IsFolder(dlg.m_sTarget))
			nImage = FAV_FOLDER_IMAGE;
		else if (_access(dlg.m_sTarget, 0) == 0)
			nImage = DOC_IMAGE;
		else
			nImage = TEXT_IMAGE;

		m_cTree.EnableFolderWatch(FALSE);
		DeleteFile(sFile);
		m_cTree.SetItemText(hItem, dlg.m_sDesc);
		m_cTree.SetItemImage(hItem, nImage, nImage);
		sFile = m_cTree.GetFileName(hItem);
		
		if (nImage == TEXT_IMAGE) {
			FILE *fp;
			fp = fopen(sFile, _T("w"));
			if (fp) {
				fputs(dlg.m_sTarget, fp);
				fclose(fp);
			}
		} else {
			CreateShortCut(dlg.m_sTarget, sFile);
		}
		m_cTree.RenumberItems(m_cTree.GetParentItem(hItem));
		m_cTree.EnableFolderWatch(TRUE);
		OnSelchangedTree1(NULL, NULL);
	}
}

void CFavoritesDlg::OnDeleteAll() 
{
	if (AfxMessageBox(IDS_DELETE_ALL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2, 0) == IDYES) {
		m_cTree.EnableFolderWatch(FALSE);
		MoveFileOrFolder(m_cTree.m_pszFolder + CString("\\*.FX*"), NULL);
		MoveFileOrFolder(m_cTree.m_pszFolder + CString("\\*.lnk"), NULL);
		m_cTree.RefreshTree();
		OnSelchangedTree1(NULL, NULL);
		m_cTree.EnableFolderWatch(TRUE);
		pMainDlg->SetHelpText(IDS_DELETE_HINT);
	}
}

void CFavoritesDlg::OnDestroy() 
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}


void CFavoritesDlg::OnKeydownTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (m_cTree.GetSelectedItem()) {
		switch (pTVKeyDown->wVKey) {
		case VK_RETURN:
			m_cTree.EditLabel(m_cTree.GetSelectedItem());
			break;
			
		case VK_DELETE:
			OnDelete();
			break;

		case VK_INSERT:
			OnNewShortcut();
			break;
		}
	}
	*pResult = 0;
}

void CFavoritesDlg::OnUpdateLinks() 
{
	CUpdateLinksDlg dlg;

	if (dlg.DoModal() == IDOK) {
		::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		m_cTree.RefreshTree(TVI_ROOT, NULL, dlg.m_iUpdateMethod + 1);
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		OnSelchangedTree1(NULL, NULL);
	}	
}

void CFavoritesDlg::OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnEditItem();
	
	*pResult = 0;
}

BOOL CFavoritesDlg::DeleteFavorite(const TCHAR *szDesc)
{
	HTREEITEM hItem = m_cTree.GetRootItem();
	if (!hItem)
		return FALSE;

	while (hItem) {
		CString s = m_cTree.GetItemText(hItem);
		if (strcmp(s, szDesc) == 0)
			break;
		hItem = m_cTree.GetNextItem(hItem, TVGN_NEXT);
	}
	if (!hItem)
		return FALSE;

	CString s = m_cTree.GetFileName(hItem);
	m_cTree.EnableFolderWatch(FALSE);
	if (MoveFileOrFolder(s, NULL)) {
		m_cTree.DeleteItem(hItem);
		pMainDlg->SetHelpText(IDS_DELETE_HINT);
	}
	m_cTree.EnableFolderWatch(TRUE);
	if (!m_cTree.GetSelectedItem())
		OnSelchangedTree1(NULL, NULL);
	if (pShData)
		m_cTree.CreateMenuResource((BYTE *) pShData->pcFavorites, sizeof(pShData->pcFavorites));
	return TRUE;
}


void CFavoritesDlg::AddFavorite(const TCHAR *pcDesc, const TCHAR *pcTarget, HTREEITEM hInsertAfter)
{
	int nImage;
	CString sDesc = EncodeFileName(pcDesc);
	CString sTarget = pcTarget;

	if (IsFolder(sTarget))
		nImage = FAV_FOLDER_IMAGE;
	else if (_access(sTarget, 0) == 0)
		nImage = DOC_IMAGE;
	else
		nImage = TEXT_IMAGE;

	HTREEITEM hParentItem = TVI_ROOT;
	int n = -1;
	if (hInsertAfter) {
		m_cTree.GetItemImage(hInsertAfter, n, n);
		hParentItem = m_cTree.GetParentItem(hInsertAfter);
	} else
		hInsertAfter = TVI_LAST;

	TCHAR szPath[MAX_PATH + 20], buf[MAX_PATH + 20];
	CString sFile = m_cTree.GetFileName(hInsertAfter, szPath);
	if (n == FOLDER_IMAGE) {
		hParentItem = hInsertAfter;
		hInsertAfter = TVI_FIRST;
		strcpy(szPath, sFile);
		n = 1;
	} else if (hInsertAfter != TVI_LAST)
		n = (int) m_cTree.GetItemData(hInsertAfter);
	else
		n = 0xf0;

	m_cTree.EnableFolderWatch(FALSE);
	if (nImage == TEXT_IMAGE) {
		FILE *fp = NULL;
		do {
			n++;
			sprintf(buf, _T("%s\\%s.FX%02x"), szPath, sDesc, n);
			if (_access(buf, 0) < 0)
				fp = fopen(buf, _T("w"));
		} while (!fp && n < 0xff);
		if (!fp)
			return;
		fprintf(fp, sTarget);
		fclose(fp);

	} else {
		sprintf(buf, _T("%s\\%s.lnk"), szPath, sDesc);
		int i = 0;
		while (_access(buf, 0) == 0) {
			sprintf(buf, _T("%s\\%s %d.lnk"), szPath, sDesc, ++i);
		}
		if (i) {
			TCHAR s[20];
			sprintf(s, _T(" %d"), i);
			sDesc += s;
		}
		CreateShortCut(sTarget, buf);
	}
	HTREEITEM hItem = m_cTree.InsertItem(DecodeFileName(sDesc), nImage, nImage, hParentItem, hInsertAfter);
	m_cTree.SetItemData(hItem, n);
	m_cTree.RenumberItems(hParentItem);
	m_cTree.SelectItem(hItem);
	m_cTree.EnableFolderWatch(TRUE);
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData) 
{
	TCHAR szDir[MAX_PATH];

	switch(uMsg) {
    case BFFM_INITIALIZED: {
       if (GetCurrentDirectory(sizeof(szDir)/sizeof(TCHAR), szDir)) {
          // WParam is TRUE since you are passing a path.
          // It would be FALSE if you were passing a pidl.
          SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)szDir);
       }
       break;
    }
    case BFFM_SELCHANGED: {
       // Set the status window to the currently selected path.
       if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir)) {
          SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
       }
       break;
    }
    default:
       break;
	}
	return 0;
}

BOOL CopyDirectoryTree(const TCHAR *from, const TCHAR *to)
{
	TCHAR buf2[MAX_PATH + 20];
	BOOL bDirCreated = FALSE;
	BOOL bRet = TRUE;
	GetCurrentDirectory(MAX_PATH, buf2);

	if (!SetCurrentDirectory(to)) {
		CreateDirectory(to, NULL);
		if (!SetCurrentDirectory(to))
			return FALSE;
		bDirCreated = TRUE;
	}
	if (!SetCurrentDirectory(from)) {
		if (bDirCreated)
			RemoveDirectory(to);
		SetCurrentDirectory(buf2);
		return FALSE;
	}

	CFileFind finder;
	BOOL bWorking = finder.FindFile(_T("*.*"));

	CString sOldDir = from;
	if (sOldDir.Right(1) != _T("\\"))
		sOldDir += _T("\\");
	CString sNewDir = to;
	if (sNewDir.Right(1) != _T("\\"))
		sNewDir += _T("\\");

	while (bWorking && bRet)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDirectory())
		{
			if (finder.GetFileName() != _T(".") && finder.GetFileName() != _T("..")) 
			{
				bRet = CopyDirectoryTree(sOldDir + finder.GetFileName(), sNewDir + finder.GetFileName());
			}
		} 
		else
		{
			bRet = CopyFile(finder.GetFileName(), sNewDir + finder.GetFileName(), FALSE);
		}
	}

	SetCurrentDirectory(buf2);
	return bRet;
}

void CFavoritesDlg::OnChangeFolder() 
{
	TCHAR buf[MAX_PATH + 20];
	BROWSEINFO bi;
	LPITEMIDLIST pItemList;
	TCHAR szDisplayName[MAX_PATH];
	memset(&bi, 0, sizeof(bi));
	CString sPrompt;
	sPrompt.LoadString(IDS_FOLDER_SELECT2);

	//pMainDlg->m_pFavDlg->m_cTree.m_pszFolder;

    bi.hwndOwner = m_hWnd; 
    bi.pidlRoot = NULL; 
    bi.pszDisplayName = szDisplayName; 
    bi.lpszTitle = sPrompt; 
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_STATUSTEXT;
	bi.lpfn = BrowseCallbackProc;

	CString sNewFolder;
	TCHAR buf2[MAX_PATH + 20];
	GetCurrentDirectory(MAX_PATH, buf2);
	SetCurrentDirectory(pMainDlg->m_pFavDlg->m_cTree.m_pszFolder);
	CoInitialize(NULL);
	if (pItemList = SHBrowseForFolder(&bi)) {
		if (SHGetPathFromIDList(pItemList, buf)) {
			UpdateData(FALSE);
			sNewFolder = buf;
		}
		LPMALLOC pMalloc;
		if (SHGetMalloc(&pMalloc) == NOERROR) 
			pMalloc->Free(pItemList);
	}

	if (sNewFolder > _T(" ")) {
		BOOL bFbxFolder = FALSE;
		SetCurrentDirectory(sNewFolder);
		CFileFind finder;
		BOOL bWorking = finder.FindFile(_T("*.*"));
		int n = 0;
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.GetFileName() != "." && finder.GetFileName() != "..") 
			{
				n++;
			}
		}

		if (n == 0 || n == 1 && _access(_T("settings.cfg"), 0) == 0) 
		{ // Selected or created an empty directory
			n = AfxMessageBox(IDS_CFG_FOLDER_EMPTY, MB_YESNO);
			if (n == IDYES) {
				// Perform a copy
				CopyDirectoryTree(pMainDlg->m_pFavDlg->m_cTree.m_pszFolder, sNewFolder);
			}
			bFbxFolder = TRUE;
		}
		else // Non-emtpy directory selected
		{
			FILE *fpcfg = fopen(_T("settings.cfg"), _T("r"));
			if (fpcfg) {
				n = 0;
				while (fgets(buf, MAX_PATH, fpcfg)) {
					if (strstr(buf, _T("nMaxRecent =")) == buf ||
						strstr(buf, _T("nPixelsLeft =")) == buf ||
						strstr(buf, _T("CurrentDir =")) == buf ||
						strstr(buf, _T("DisplayButtons =")) == buf)
						n++;
					if (n > 3) {
						bFbxFolder = TRUE;
						break;
					}
				}
				fclose(fpcfg);
			}
			if (!bFbxFolder) 
			{
				// Can't use this folder, not a previous FbX configuration folder
				AfxMessageBox(IDS_CFG_FOLDER_NOT_EMPTY);
			}

		}

		if (bFbxFolder) 
		{
			pMainDlg->SaveStatus(); // Save the old one
			strcpy(pMainDlg->m_pFavDlg->m_cTree.m_pszFolder, sNewFolder);
			pMainDlg->ReadStatus();
			pMainDlg->SaveStatus();
			::SetCursor(::LoadCursor(NULL, IDC_WAIT));
			m_cTree.RefreshTree(TVI_ROOT, NULL, TRUE);
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			OnSelchangedTree1(NULL, NULL);
		}

	}

	SetCurrentDirectory(buf2);

}
