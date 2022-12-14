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

// FileBxDlg.cpp : implementation file
//

#include "stdafx.h"
#include <multimon.h>
#include "FileBX.h"

#include "AboutDlg.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "KeySetDlg.h"
#include "OptionsDlg.h"
#include "ExceptionsDlg.h"
#include "FileBxDlg.h"
#include "ItemDlg.h"
#include "DFH/DFH_shared.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define WM_TRAYICONMSG (WM_USER + 11)
const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));
static NOTIFYICONDATA s_nid = { sizeof( NOTIFYICONDATA ), 0 };

typedef void (CALLBACK *MYCALLBACK) ();
typedef DF_SHARED_DATA * (CALLBACK *MYCALLBACK1) (HINSTANCE, DWORD);

DF_SHARED_DATA *pShData = NULL;
DF_SHARED_DATA_COPY *pShDataCopy = NULL; // for 64-bit to 32 bit communication

MYCALLBACK pReleaseMsgHooks = NULL;
MYCALLBACK1 pInstallMsgHooks = NULL;
HINSTANCE hInstDLL = NULL;

CFileBxDlg *pMainDlg = NULL;
CString g_sBrowseDir;

UINT FBX_QUIT =  RegisterWindowMessage("FileBox eXtender Quit Message");


#ifndef MSGFLT_ADD
# define MSGFLT_ADD 1
# define MSGFLT_REMOVE 2
#endif
typedef WINUSERAPI BOOL (WINAPI *ChangeWindowMessageFilter_t) (UINT message, DWORD dwFlag);
ChangeWindowMessageFilter_t ChangeWindowMessageFilter_ = 
	(ChangeWindowMessageFilter_t) GetProcAddress(GetModuleHandle(_T("user32.dll")), "ChangeWindowMessageFilter"); 

/////////////////////////////////////////////////////////////////////////////
// CFileBxDlg dialog


CFileBxDlg::CFileBxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFileBxDlg::IDD, pParent)
	, m_pHeaderDlg(NULL)
	, m_nTabPosX(-1)
{
	//{{AFX_DATA_INIT(CFileBxDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	HDC hdc = ::GetDC(NULL);
	//if (::GetDeviceCaps(hdc, BITSPIXEL) <= 8)
		m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//else
	//	m_hIcon = AfxGetApp()->LoadIcon(IDI_MAINFRAME_HICOLOR);
	::ReleaseDC(NULL, hdc);
	m_bHookInstalled = FALSE;
	m_pSetDlg = NULL;
	m_pOptsDlg = NULL;
	m_pFavDlg = NULL;
	m_pKeyDlg = NULL;
	for (int i = 0; i < MAX_TAB_PAGES; i++)
		m_pPage[i] = NULL;

	m_hWndBar = NULL;
}

CFileBxDlg::~CFileBxDlg()
{
	int i;
	for (i = 0; i < m_nTabs; i++) 
		delete m_pPage[i];
}

void CFileBxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileBxDlg)
	DDX_Control(pDX, IDC_HELP_TEXT, m_cHelpText);
	DDX_Control(pDX, IDC_TAB1, m_cTab);
	DDX_Control(pDX, IDC_HELP_FBX, m_cHelp);
	DDX_Control(pDX, IDC_EXIT, m_cExit);
	DDX_Control(pDX, IDC_ABOUT, m_cAbout);
	DDX_Control(pDX, IDC_CLOSE, m_cClose);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_TOGGLE_INFO, m_cToggleInfo);
}


BEGIN_MESSAGE_MAP(CFileBxDlg, CDialog)
	//{{AFX_MSG_MAP(CFileBxDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_CLOSE, OnClose)
	ON_BN_CLICKED(IDC_HELP_FBX, OnHelpFbx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_EXIT, OnExitButton)
	ON_COMMAND(ID_EXIT, OnExit)
	ON_COMMAND(ID_OPEN, OnOpen)
	ON_WM_DESTROY()
	ON_WM_HELPINFO()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_TRAYICONMSG, OnTrayIconMsg)
	ON_WM_ACTIVATE()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCHITTEST()
	ON_WM_QUERYENDSESSION()
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, OnTaskBarCreated)
	ON_BN_CLICKED(IDC_TOGGLE_INFO, &CFileBxDlg::OnBnClickedToggleInfo)
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_WM_WINDOWPOSCHANGED()
	ON_REGISTERED_MESSAGE(FBX_QUIT, OnQuit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileBxDlg message handlers
void CFileBxDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else if ((nID & 0xFFF0) == SC_CLOSE) {
		OnClose();
	} else {
		CDialog::OnSysCommand(nID, lParam);
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFileBxDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CFileBxDlg::ResizeControls()
{
	if (!m_cExit.m_hWnd || !IsWindow(m_cExit.m_hWnd))
		return;
	CRect rctFbX, rHlpTxt, rBtn, rPage, rTab;
	GetClientRect(&rctFbX);
	m_cTab.GetWindowRect(&rTab);
	ScreenToClient(&rTab);
	if (m_nTabPosX < 0)
		m_nTabPosX = rTab.left;

	if (m_pHeaderDlg != NULL && m_pHeaderDlg->IsWindowVisible())
	{
		CRect rHead;
		m_pHeaderDlg->GetWindowRect(&rHead);
		ScreenToClient(&rHead);
		rTab.left = m_nTabPosX;
		m_pHeaderDlg->SetWindowPos(&wndBottom, 2, 3, rTab.left-6, rctFbX.bottom - 4, SWP_SHOWWINDOW);
	} else
		rTab.left = 2;

	rTab.right = rctFbX.right - 2;
	m_cClose.GetWindowRect(&rBtn);
	m_cHelpText.GetWindowRect(&rHlpTxt);
	int n = rctFbX.bottom - rHlpTxt.Height() - rBtn.Height() - 2;
	int d = rTab.right; // rctFbX.right - rTab.left;

	m_cClose.SetWindowPos(NULL, d - rBtn.Width(), n, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
	m_cAbout.SetWindowPos(NULL, d - 2*rBtn.Width() - 3, n, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
	m_cHelp.SetWindowPos(NULL, d - 3*rBtn.Width() - 6, n, 0, 0, 
		SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

	m_cToggleInfo.SetWindowPos(NULL, rTab.left, n, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
	m_cToggleInfo.GetWindowRect(&rBtn);
	ScreenToClient(&rBtn);
	m_cExit.SetWindowPos(NULL, rBtn.right + 3, n, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

	// m_cHelpText.GetWindowRect(&rHlpTxt);
	ScreenToClient(&rHlpTxt);
	rHlpTxt.right = rctFbX.right - 18;
	int h = rHlpTxt.Height();
	rHlpTxt.top = rctFbX.bottom - h;
	rHlpTxt.bottom = rHlpTxt.top + h;
	m_cHelpText.SetWindowPos(NULL, rTab.left, rHlpTxt.top, rHlpTxt.Width(), rHlpTxt.Height(), 
		SWP_NOZORDER | SWP_DRAWFRAME);

	// rTab.right = rctFbX.right - rTab.left;
	rTab.bottom = rctFbX.bottom - rHlpTxt.Height() - rBtn.Height() - 6;
	m_cTab.SetWindowPos(NULL, rTab.left, rTab.top, rTab.Width(), rTab.Height(), 
		SWP_NOZORDER | SWP_DRAWFRAME);

	int i;
	for (i = 0; i < m_nTabs; i++) {
		m_pPage[i]->GetWindowRect(&rPage);
		ScreenToClient(&rPage);
		rPage.left = rTab.left + 2;
		rPage.right = rTab.right - 2; //rctFbX.right - rPage.left;
		rPage.top = rTab.top + 22;
		rPage.bottom = rctFbX.bottom - rHlpTxt.Height() - rBtn.Height() - 10;
		m_pPage[i]->SetWindowPos(NULL, rPage.left, rPage.top, rPage.Width(), rPage.Height(), 
			SWP_NOZORDER);
	}
	InvalidateRect(&rctFbX, TRUE);
	m_cClose.Invalidate();
	m_cHelp.Invalidate();
	m_cExit.Invalidate();
	m_cHelpText.Invalidate();
}

void CFileBxDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if (nType == SIZE_MINIMIZED) {
		//OnClose();
		//return;
	}
	ResizeControls();

}

void CFileBxDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

}


void CFileBxDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	//if (m_hWndBar)
	//{
	//	BOOL bVis = bShow && !::IsIconic(m_hWnd);
	//	CRect r, rParent;
	//	::GetWindowRect(m_hWndBar, &r);
	//	::GetWindowRect(m_hWnd, &rParent);
	//	::MoveWindow(m_hWndBar, rParent.right - r.Width() - 100, rParent.top, r.Width(), r.Height(), TRUE);
	//	::ShowWindow(m_hWndBar, bVis ? SW_SHOWNA : SW_HIDE);
	//}
}

void CFileBxDlg::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	CDialog::OnWindowPosChanged(lpwndpos);

	//if (m_hWndBar)
	//{
	//	BOOL bVis = ::IsWindowVisible(m_hWnd) && !::IsIconic(m_hWnd);
	//	CRect r, rParent;
	//	::GetWindowRect(m_hWndBar, &r);
	//	::GetWindowRect(m_hWnd, &rParent);
	//	::MoveWindow(m_hWndBar, rParent.right - r.Width() - 100, rParent.top, r.Width(), r.Height(), TRUE);
	//	::ShowWindow(m_hWndBar, bVis ? SW_SHOWNA : SW_HIDE);
	//}
}

void CFileBxDlg::OnWindowPosChanging( WINDOWPOS* lpwndpos )
{
	int xmin, ymin;
	CRect r;
	m_cExit.GetWindowRect(&r);
	xmin = 6*r.Width();
	ymin = 16*r.Height()+8;
	if (m_pHeaderDlg != NULL && m_pHeaderDlg->IsWindowVisible())
	{
		m_pHeaderDlg->GetWindowRect(&r);
		xmin += r.Width() + 3;
	}
	m_cTab.GetWindowRect(&r);
	ScreenToClient(&r);
	ymin += r.top;
	if (lpwndpos->cx < xmin)
		lpwndpos->cx = xmin;
	if (lpwndpos->cy < ymin)
		lpwndpos->cy = ymin;

	CDialog::OnWindowPosChanging(lpwndpos);

}

LRESULT CFileBxDlg::OnNcHitTest( CPoint point )
{
	CPoint pt = point;
	CRect r;
	ScreenToClient(&pt);
	GetClientRect(&r);
	r.top = r.bottom - 16;
	r.left = r.right - 16;
	if (PtInRect(&r, pt)) {
		return HTBOTTOMRIGHT;
	}

	return CDialog::OnNcHitTest(point);
}



void CFileBxDlg::OnAbout() 
{
#ifdef _DEBUG
	if (pShData && IsHookInstalled()) {
		pShData->m_bFindCtl = TRUE;
		return;
	}
#endif
	CAboutDlg dlgAbout;
	dlgAbout.DoModal();	
}

void CFileBxDlg::OnClose() 
{
	SaveStatus();	
	WinHelp(0L, HELP_QUIT);
	ShowWindow(SW_HIDE);
}

BOOL CFileBxDlg::OnQueryEndSession() 
{
	SaveStatus();	
	return TRUE;
}



void CFileBxDlg::OnHelpFbx() 
{
	/* - Test of A-Links
	HH_AKLINK link;
   link.cbStruct =     sizeof(HH_AKLINK) ;
   link.fReserved =    FALSE ;
   link.pszKeywords =  "EDCA6D78" ;
   link.pszUrl =       NULL ;
   link.pszMsgText =   NULL ;
   link.pszMsgTitle =  NULL ;
   link.pszWindow =    NULL ;
   link.fIndexOnFail = TRUE ;

	::HtmlHelp(NULL, CString(theApp.m_pszHelpFilePath), HH_ALINK_LOOKUP, (DWORD) &link);
	return;
	*/

	static char *pcTab[] = {
		"favorites_tab",
		"settings_tab",
		"fileboxes_tab",
		"keys_menu_tab",
		"exceptions_tab",
		"license_tab",
		"",
		"",
		"",
		"",
	};

	int nTab = m_cTab.GetCurSel();
	::HtmlHelp(NULL, CString(theApp.m_pszHelpFilePath) + "::/html/" + CString(pcTab[nTab]) + ".html", HH_DISPLAY_TOPIC, NULL);	
	//::HtmlHelp(m_hWnd, CString(theApp.m_pszHelpFilePath) + "::/html/welcome.html>popup", HH_DISPLAY_TOPIC, NULL);	
	// If I use m_hWnd, the help window is always on top of FbX.

	//theApp.WinHelp(IDD_FILEBX + 0x20000, HELP_CONTEXT);
}

static void GetDirName(TCHAR *buf2, TCHAR *name)
{
	TCHAR *pc;
	pc = buf2 + strlen(buf2) - 1;
	if (*pc == '\\')
		*pc = 0;
	if (pShData->bRecentPath) {
		strcpy(name, buf2);
	} else {
		while (pc > buf2 && *pc != '\\')
			pc--;
		if (*pc == '\\') pc++;
		strcpy(name, pc);
		if (buf2[1] == ':')
			wsprintf(name + strlen(name), _T("  (%c:)"), islower(*buf2) ? _toupper(*buf2) : *buf2);
		else if (buf2[1] == '\\') {
			for (pc = buf2+2; pc < buf2+MAX_PATH && *pc != '\\'; pc++);
			if (pc < buf2+MAX_PATH) {
				for (pc++; pc < buf2+MAX_PATH && *pc != '\\'; pc++);
				if (pc < buf2+MAX_PATH) {
					*pc = 0;
					wsprintf(name + strlen(name), _T("  (%s)"), buf2);
				}
			}
		}
	}
}


LRESULT CFileBxDlg::OnTaskBarCreated(WPARAM wp, LPARAM lp)
{
	if (m_pSetDlg->m_cTrayIcon.GetCheck() || !m_bHookInstalled) {
		s_nid.hWnd = NULL;
		OnTrayIconMsg(0, 0); // show tray icon
	}
    return 0;
}


LRESULT CFileBxDlg::OnTrayIconMsg(WPARAM wParam, LPARAM lParam)
{
	int iRet = 0;
	
	unsigned u = 0;
	if (wParam == 0) {
		
		if (m_pSetDlg->m_cTrayIcon.GetCheck() || 
			(pShData->dwButtons & 0x7fff & (~BTN_PUSHPIN)) == 0) 
		{
			// Set Tray Icon
			BOOL bModify = FALSE;
			if (s_nid.hWnd) {
				bModify = TRUE;
			}


			s_nid.cbSize = sizeof( NOTIFYICONDATA );
			s_nid.hWnd = m_hWnd;
			s_nid.uID = 100;
			s_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			s_nid.uCallbackMessage = WM_TRAYICONMSG;

			if (pShData->dwButtons & 0x7fff)
				s_nid.hIcon = (HICON) LoadImage(AfxGetResourceHandle(), 
					MAKEINTRESOURCE(IDR_MAINFRAME), 
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			else
				s_nid.hIcon = (HICON) LoadImage(AfxGetResourceHandle(), 
					MAKEINTRESOURCE(IDR_DISABLED), 
					IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			
			strcpy( s_nid.szTip, _T("FileBox eXtender") );

			Shell_NotifyIcon( bModify ? NIM_MODIFY : NIM_ADD, &s_nid );
			return 0;
		} else {
			if (s_nid.hWnd) {
				// Remove Tray Icon
				Shell_NotifyIcon( NIM_DELETE, &s_nid );
				s_nid.hWnd = NULL;
			}
			return 0;
		}
	} else if (wParam == 1) {
		if ( !s_nid.hWnd )
		  return 0;
		// Remove Tray Icon
		Shell_NotifyIcon( NIM_DELETE, &s_nid );
		s_nid.hWnd = NULL;
		return 0;
	}

	switch ( lParam /* mouse event */ ) {
	case WM_LBUTTONDOWN:
		{
			bool bDelete = FALSE;
			if (!pShData) {
				pShData = new DF_SHARED_DATA;
				memset(pShData, 0, sizeof(DF_SHARED_DATA));
				bDelete = TRUE;
			}
			BYTE *pMenuBuffer = (BYTE *) pShData->pcFavorites;
			if (m_pFavDlg->m_cTree.CreateMenuResource(pMenuBuffer, sizeof(pShData->pcFavorites))) {
				HMENU hMenu = LoadMenuIndirect((MENUTEMPLATE *) pMenuBuffer);
				if (hMenu) {
					if (pShData->nRecent) {
						TCHAR buf2[MAX_PATH + 20], name[MAX_PATH];

						HMENU hRecentMenu = CreatePopupMenu();
						FillRecentFoldersBuffer();
						for (int i = 0; i < pShData->nRecent; i++) {
							strcpy(buf2, pShData->sRecent[i]);
							GetDirName(buf2, name);
							AppendMenu(hRecentMenu, MF_STRING, 10000+i, name);
						}
						CString s;
						s.LoadString(IDS_RECENT_FOLDERS);
						::InsertMenu(hMenu, 1, MF_BYCOMMAND | MF_STRING | MF_POPUP, 
							(UINT_PTR) hRecentMenu, s);
					}
					
					HMENU hSup = ::CreateMenu();
					::AppendMenu(hSup, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, _T("R"));
					POINT mouse;
					::GetCursorPos( &mouse );
					CRect rct;
					::SetMenuDefaultItem(hMenu, 1, FALSE);
					SetForegroundWindow();
					iRet = ::TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, mouse.x, mouse.y, m_hWnd, NULL);
					DestroyMenu(hSup);

					if (iRet >= 10000) {
						ShellExecute(m_hWnd, (pShData->dwButtons & FLD_EXPLORE) ? _T("explore") : _T("open"), 
							pShData->sRecent[iRet-10000], NULL, _T(".\\"), SW_SHOW);
						
					} else if (iRet >= 10) {
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

					} else if (iRet > 0) {
						ShowWindow(SW_RESTORE);
						::SetForegroundWindow( m_hWnd );
						m_cTab.SetCurSel(iRet - 1);
						OnSelchangeTab1(NULL, NULL);

					} else {
						ScreenToClient(&mouse);
						PostMessage(WM_LBUTTONUP, 0, mouse.x | (mouse.y << 16));
					}
				}
			}
			if (bDelete) {
				delete pShData;
				pShData = NULL;
			}
		}

		break;

	case WM_RBUTTONDOWN:
		POINT mouse;
		::GetCursorPos( &mouse );
		CMenu cMenu, *pMenu = NULL;
		cMenu.LoadMenu(IDR_TRAY_MENU);
		pMenu = cMenu.GetSubMenu(0);
		::SetMenuDefaultItem(pMenu->m_hMenu, pMenu->GetMenuItemCount() - 1, TRUE );
		SetForegroundWindow();
		PostMessage(WM_RBUTTONUP, 0, 0);
		iRet = pMenu->TrackPopupMenu(TPM_RIGHTBUTTON|TPM_RIGHTALIGN, mouse.x, mouse.y, this, NULL);
		cMenu.DestroyMenu();
		break;
	}
	return 0;
}


void CFileBxDlg::OnActivate( UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	static BOOL bPreventRecursion = FALSE;
	if (bPreventRecursion)
		return;
	bPreventRecursion =	TRUE;

	unsigned u = 0;

	if (nState == WA_INACTIVE) {
		if (pShData) {
			if (theApp.m_bAddToUninstall) {
				theApp.m_bAddToUninstall = FALSE;
				// Created for the first time, add to INSTALL.LOG
				CString fname = theApp.m_sProgDir + "INSTALL.LOG";
				FILE *fp = fopen(fname, _T("a+"));
				if (fp) {
					fprintf(fp, _T("\nNon-System File: "));
					fprintf(fp, _T("\nFile Tree: %s\\*.*"), m_pFavDlg->m_cTree.m_pszFolder);
					fclose(fp);
				}
			}
			m_pKeyDlg->SetKeys();
			m_pFavDlg->m_cTree.CreateMenuResource((BYTE *) pShData->pcFavorites, sizeof(pShData->pcFavorites));
		}
	} 
	bPreventRecursion =	FALSE;
}


void CFileBxDlg::SetHelpText(int iHelpId)
{
	CString s;
	if (iHelpId)
		s.LoadString(iHelpId);
	m_cHelpText.SetWindowText(s);
	m_cHelpText.Invalidate();
	m_cHelpText.UpdateWindow();
}

void CFileBxDlg::SaveStatus()
{
	TCHAR fname[MAX_PATH + 20];
	int i;
	theApp.RegPutString(_T("Settings"), _T("CfgDir"), m_pFavDlg->m_cTree.m_pszFolder);
	strcpy(fname, m_pFavDlg->m_cTree.m_pszFolder);
	strcat(fname, _T("\\settings.cfg"));
	if (!pShData)
		return;
	FILE *fp = fopen(fname, _T("w"));
	if (!fp)
		return;

	fprintf(fp, _T("nMaxRecent = %d\n"), pShData->nMaxRecent);
	fprintf(fp, _T("nPixelsLeft = %d\n"), pShData->nPixelsLeft);
	fprintf(fp, _T("nMaxPixelsLeft = %d\n"), pShData->nMaxPixelsLeft);
	//GetCurrentDirectory(MAX_PATH, fname);
	fprintf(fp, _T("CurrentDir = %s\n"), g_sBrowseDir);
	//fprintf(fp, "ButtonsEnabled = %d\n", m_pSetDlg->m_cEnableFBX.GetCheck());
	fprintf(fp, _T("DisplayButtons = %d\n"), pShData->dwButtons);
	fprintf(fp, _T("TrayIcon = %d\n"), m_pSetDlg->m_cTrayIcon.GetCheck());
	fprintf(fp, _T("RecentPath = %d\n"), m_pKeyDlg->m_cRecentPath.GetCheck());
	fprintf(fp, _T("RecentSort = %d\n"), m_pKeyDlg->m_cRecentSort.GetCheck());
	fprintf(fp, _T("RecentFromWin = %d\n"), m_pKeyDlg->m_cRecentFromWin.GetCheck());
	fprintf(fp, _T("SysTopmost = %d\n"), m_pKeyDlg->m_cSysTopmost.GetCheck());
	fprintf(fp, _T("SysRollup = %d\n"), m_pKeyDlg->m_cSysRollup.GetCheck());
	fprintf(fp, _T("DetailesView = %d\n"), pShData->dwDetailsView);
	fprintf(fp, _T("ResizeFlags = %d\n"), pShData->nResizeFlags);
	fprintf(fp, _T("ResizeHeight = %d\n"), pShData->nResizeHeight);
	fprintf(fp, _T("ResizeWidth = %d\n"), pShData->nResizeWidth);
	fprintf(fp, _T("wFavKey = %d\n"), pShData->wFavKey);
	fprintf(fp, _T("wFavMod = %d\n"), pShData->wFavMod);
	fprintf(fp, _T("wRecKey = %d\n"), pShData->wRecKey);
	fprintf(fp, _T("wRecMod = %d\n"), pShData->wRecMod);
	// fprintf(fp, "bFavAddNum = %d\n", pShData->bFavAddNum); // not used
	fprintf(fp, _T("bRecAddNum = %d\n"), pShData->bRecAddNum);
	fprintf(fp, _T("bRolledTopmost = %d\n"), pShData->bRolledTopmost);
	fprintf(fp, _T("nWantClickDir = %d\n"), pShData->nWantClickDir);

	fprintf(fp, _T("--- Recent ---\n"));
	for (i = 0; i < pShData->nRecent; i++) {
		fprintf(fp, _T("%s\n"), pShData->sRecent[i]);
	}
	fprintf(fp, _T("--- Recent ---\n"));

	fprintf(fp, _T("--- Exceptions ---\n"));
	for (i = 0; i < pShData->nExceptions; i++) {
		fprintf(fp, _T("%s\n"), pShData->sExceptions[i]);
	}
	fprintf(fp, _T("--- Exceptions ---\n"));

	fclose(fp);
	if (pShDataCopy && pShData->dwSize == pShDataCopy->dwSize)
		memcpy(pShDataCopy, pShData, SHMEMSIZE_COPY);
}

void CFileBxDlg::ReadStatus()
{
	TCHAR buf[MAX_PATH + 256], *pc;
	strcpy(buf, m_pFavDlg->m_cTree.m_pszFolder);
	strcat(buf, _T("\\settings.cfg"));
	if (!pShData)
		return;
	pShData->nRecent = 0;
	pShData->nExceptions = 0;
	pShData->nMaxRecent = 8;
	pShData->nPixelsLeft = 2;
	pShData->nResizeFlags = 1;
	pShData->nResizeWidth = 125;
	pShData->nResizeHeight = 125;
	pShData->dwButtons = BTN_PUSHPIN | BTN_ROLLUP | BTN_FAV_FB | BTN_REC_FB | BTN_FAV_EX | BTN_REC_EX | FLD_EXPLORE;
	pShData->nWantClickDir = 1;

	// Needed for resizing of windows
	pShData->nMaxWidth = (int)(.95 * GetSystemMetrics(SM_CXSCREEN));
	pShData->nMaxHeight = (int)(.95 * GetSystemMetrics(SM_CYSCREEN)) - 32;
	pShData->nDeskRight = GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	pShData->nDeskBottom = GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN) - 32;


	// m_pSetDlg->m_cEnableFBX.SetCheck(TRUE);
	m_pSetDlg->m_cTrayIcon.SetCheck(TRUE);
	CString sText;
	sText.LoadString(IDS_ADD);
	strcpy(pShData->szAdd, sText);
	sText.LoadString(IDS_REMOVE);
	strcpy(pShData->szRemove, sText);
	sText.LoadString(IDS_ON_TOP);
	strcpy(pShData->szOnTop, sText);
	sText.LoadString(IDS_ROLL_UP);
	strcpy(pShData->szRollUp, sText);

	FILE *fp = fopen(buf, _T("r"));
	if (fp) {
		while (fgets(buf, MAX_PATH + 20, fp)) {
			pc = buf + strlen(buf)-1;
			while (pc >= buf && *pc < ' ')
				*pc-- = 0;	// strip new line characters
			if (strstr(buf, _T("nMaxRecent = ")) == buf) {
				pShData->nMaxRecent = atoi(buf + 12);
				if (pShData->nMaxRecent > MAX_RECENT)
					pShData->nMaxRecent = MAX_RECENT;

			} else if (strstr(buf, _T("CurrentDir = ")) == buf) {
				g_sBrowseDir = buf + 13;
			//	SetCurrentDirectory(buf + 13);

			} else if (strstr(buf, _T("nPixelsLeft = ")) == buf) {
				pShData->nPixelsLeft = atoi(buf + 13);
			} else if (strstr(buf, _T("DisplayButtons = ")) == buf) {
				pShData->dwButtons = atoi(buf + 17);

			} else if (strstr(buf, _T("TrayIcon = ")) == buf) {
				m_pSetDlg->m_cTrayIcon.SetCheck(atoi(buf + 11));

			} else if (strstr(buf, _T("nMaxPixelsLeft = ")) == buf) {
				pShData->nMaxPixelsLeft = atoi(buf + 16);

			} else if (strstr(buf, _T("RecentPath = ")) == buf) {
				pShData->bRecentPath = (bool) atoi(buf + 13);

			} else if (strstr(buf, _T("RecentSort = ")) == buf) {
				pShData->bSortRecent = (bool) atoi(buf + 13);

			} else if (strstr(buf, _T("RecentFromWin = ")) == buf) {
				pShData->bRecentFromWin = (bool) atoi(buf + 16);

			} else if (strstr(buf, _T("SysTopmost = ")) == buf) {
				pShData->bSysTopmost = (bool) atoi(buf + 13);

			} else if (strstr(buf, _T("SysRollup = ")) == buf) {
				pShData->bSysRollup = (bool) atoi(buf + 12);

			} else if (strstr(buf, _T("DetailesView = ")) == buf) {
				pShData->dwDetailsView = atoi(buf + 15);

			} else if (strstr(buf, _T("ResizeFlags = ")) == buf) {
				pShData->nResizeFlags = atoi(buf + 14);

			} else if (strstr(buf, _T("ResizeHeight = ")) == buf) {
				pShData->nResizeHeight = atoi(buf + 15);

			} else if (strstr(buf, _T("ResizeWidth = ")) == buf) {
				pShData->nResizeWidth = atoi(buf + 14);

			} else if (strstr(buf, _T("wFavKey = ")) == buf) {
				pShData->wFavKey = atoi(buf + 10);

			} else if (strstr(buf, _T("wFavMod = ")) == buf) {
				pShData->wFavMod = atoi(buf + 10);

			} else if (strstr(buf, _T("wRecKey = ")) == buf) {
				pShData->wRecKey = atoi(buf + 10);

			} else if (strstr(buf, _T("wRecMod = ")) == buf) {
				pShData->wRecMod = atoi(buf + 10);

			//} else if (strstr(buf, "bFavAddNum = ") == buf) { // not used
			//	pShData->bFavAddNum = (bool) atoi(buf + 13);

			} else if (strstr(buf, _T("bRecAddNum = ")) == buf) {
				pShData->bRecAddNum = (bool) atoi(buf + 13);

			} else if (strstr(buf, _T("bRolledTopmost = ")) == buf) {
				pShData->bRolledTopmost = (bool) atoi(buf + 17);

			} else if (strstr(buf, _T("nWantClickDir = ")) == buf) {
				pShData->nWantClickDir = atoi(buf + 16);

			} else if (strstr(buf, _T("--- Recent ---")) == buf) {
				while (fgets(buf, MAX_PATH + 20, fp)) {
					pc = buf + strlen(buf)-1;
					while (pc >= buf && *pc < ' ')
						*pc-- = 0;	// strip new line characters
					if (strstr(buf, _T("--- Recent ---")) == buf)
						break;
					if (pShData->nRecent < pShData->nMaxRecent)
						strcpy(pShData->sRecent[pShData->nRecent++], buf);
				}
			} else if (strstr(buf, _T("--- Exceptions ---")) == buf) {
				while (fgets(buf, MAX_PATH + 20, fp)) {
					pc = buf + strlen(buf)-1;
					while (pc >= buf && *pc < ' ')
						*pc-- = 0;	// strip new line characters
					if (strstr(buf, _T("--- Exceptions ---")) == buf)
						break;
					strlwr(buf);
					if (pShData->nExceptions < MAX_EXCEPTIONS) {
						if (buf[1] <= ' ') {
							// Translate to ver. 1.5 exception format
							int nEx = 0;
							if (buf[0] == '0') // do not handle at all
								nEx = 1;
							else if (buf[0] == 'v') // resize vert. only
								nEx |= FB_RESIZE_VONLY;
							else if (buf[0] == 'h')
								nEx |= FB_RESIZE_HONLY;
							sprintf(pShData->sExceptions[pShData->nExceptions++], _T("%02x00 %s"), nEx, buf+2);
						} else
							strcpy(pShData->sExceptions[pShData->nExceptions++], buf);
					}
				}
			}
		}
		fclose(fp);
	}

	pShData->bSwitchYZ = getenv("FBX_SWITCH_YZ") ? TRUE : FALSE;

	m_pKeyDlg->InitData();
	m_pOptsDlg->InitData();
	m_pSetDlg->InitData();
}


void CFileBxDlg::ToggleHooks()
{
	if (m_bHookInstalled) {
		if (pReleaseMsgHooks) {
			pReleaseMsgHooks();
			pInstallMsgHooks = NULL;
			pReleaseMsgHooks = NULL;
		}
		m_bHookInstalled = FALSE;

	} else {
		if (!hInstDLL)
			hInstDLL = LoadLibrary(_T("FileBXH.dll"));
		if (hInstDLL) {
			pInstallMsgHooks = (MYCALLBACK1) GetProcAddress(hInstDLL, "InstallMsgHooks"); 
			pReleaseMsgHooks = (MYCALLBACK) GetProcAddress(hInstDLL, "ReleaseMsgHooks");
		}

		if (pInstallMsgHooks && pReleaseMsgHooks && 
			(BOOL) (pShData = pInstallMsgHooks(hInstDLL, 0))) 
		{
			pShData->hDFhWnd = m_hWnd;
			m_bHookInstalled = TRUE;
		} else {
			Msg(_T("pInstallMsgHooks=%X, pReleaseMsgHooks=%X, pShData=%X"), pInstallMsgHooks, pReleaseMsgHooks, pShData);
			if (pReleaseMsgHooks)
				pReleaseMsgHooks();
			m_bHookInstalled = FALSE;
			pInstallMsgHooks = NULL;
			pReleaseMsgHooks = NULL;
			AfxMessageBox(IDS_VIRUS);
			ExitProcess(1);
		}
	}

	// m_pSetDlg->m_cEnableFBX.SetCheck(m_bHookInstalled);
	m_pSetDlg->m_cTrayIcon.EnableWindow(m_bHookInstalled);
	if (m_pSetDlg->m_cTrayIcon.GetCheck() || !m_bHookInstalled)
		OnTrayIconMsg(0, 0); // Set tray icon;
	else
		OnTrayIconMsg(1, 0); // Remove tray icon	
}


BOOL CFileBxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	pMainDlg = this;
	m_pHeaderDlg = new HeaderDlg(this);
	m_pHeaderDlg->Create(IDD_HEADER, this);
		m_pHeaderDlg->SetWindowPos(&wndBottom, 4, 4, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Set the tabs of m_cTab control
    TCITEM tc;
	memset(&tc, 0, sizeof(tc));
	tc.mask = TCIF_TEXT;
    CString sText;
	m_nTabs = 6;

	m_pPage[1] = new CSettingsDlg(); m_pPage[1]->Create(IDD_SETTINGS, (CWnd *) this);
	m_pPage[2] = new COptionsDlg(); m_pPage[2]->Create(IDD_OPTIONS, (CWnd *) this);
	m_pPage[3] = new CKeySetDlg(); m_pPage[3]->Create(IDD_KEY_SETTINGS, (CWnd *) this);
	// m_pPage[1], m_pPage[2], m_pPage[3] must be added first above!
	m_pPage[0] = new CFavoritesDlg(); m_pPage[0]->Create(IDD_FAVORITES, (CWnd *) this);
	m_pPage[4] = new CExceptionsDlg(); m_pPage[4]->Create(IDD_EXCEPTIONS, (CWnd *) this);

	m_pPage[LIC_PAGE] = NULL;
	m_nTabs--;

	int i;
	for (i = 0; i < m_nTabs; i++) {
		memset(&tc, 0, sizeof(tc));
		tc.mask = TCIF_PARAM | TCIF_TEXT;
		m_pPage[i]->GetWindowText(sText);
		tc.pszText = (TCHAR*)(const TCHAR *) sText;
		if (i)
			m_pPage[i]->ShowWindow(SW_HIDE);
		tc.lParam = (LPARAM) m_pPage[i];
		m_cTab.InsertItem(i, &tc);
	}
	m_cTab.SetWindowPos(&wndBottom, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
	m_pPage[0]->ShowWindow(SW_SHOW);
	m_pFavDlg->m_cTree.SetFocus();

	//CRect r;
	//GetWindowRect(&r);
	//m_hWndBar = CreateWindowEx(
	//	// 0x02000000L | 0x00080000 |
	//	// WS_EX_COMPOSITED | WS_EX_LAYERED |
	//	WS_EX_NOACTIVATE | WS_EX_NOPARENTNOTIFY | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
	//	_T("#32770"),
	//	_T(""),
	//	WS_BORDER | WS_POPUP,
	//	r.right - 200, r.top,
	//	100, 20,
	//	m_hWnd,
	//	NULL,
	//	NULL,
	//	NULL
	//	);

	ResizeControls();
	UpdateData(FALSE);

	if (pShData->dwButtons & 0x7fff)
		ToggleHooks();
	else {
		OnTrayIconMsg(0, 0);
	}

	SetHelpText();

	if (ChangeWindowMessageFilter_)
		BOOL bbb = ChangeWindowMessageFilter_(WM_COMMAND, MSGFLT_ADD);

	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CFileBxDlg::OnSelchangeTab1(NMHDR* , LRESULT* pResult) 
{
	int i;
    int iSel = m_cTab.GetCurSel();
	if (iSel < 0)
		return;
	for (i = 0; i < m_nTabs; i++) {
		m_pPage[i]->ShowWindow(SW_HIDE);
	}
	
	m_pPage[iSel]->ShowWindow(SW_SHOW);
	m_pPage[iSel]->SetFocus();
	
	if (pResult)
		*pResult = 0;
}


void CFileBxDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CRect r;
		GetClientRect(&r);
		CPaintDC dc(this); // device context for painting
		DWORD dwColorDark = GetSysColor(COLOR_3DSHADOW);
		DWORD dwColorLight = GetSysColor(COLOR_3DHILIGHT);
		CPen cPenDk(PS_SOLID, 2, dwColorDark);
		CPen cPenLt(PS_SOLID, 0, dwColorLight);
		CPen *pOldPen = dc.SelectObject(&cPenDk);
		int x = r.right - 1;
		int y = r.bottom - 1;

		for (int i = 0; i < 16; i += 4) {
			dc.SelectObject(&cPenDk);
			dc.MoveTo(x-i-1, y);
			dc.LineTo(x, y-i-1);
			dc.SelectObject(&cPenLt);
			dc.MoveTo(x-i-2, y);
			dc.LineTo(x, y-i-2);
		}

		dc.SelectObject(pOldPen);
		cPenDk.DeleteObject();
		cPenLt.DeleteObject();
		CDialog::OnPaint();
	}
}

void CFileBxDlg::OnBnClickedToggleInfo()
{
	if (m_pHeaderDlg) {
		BOOL bHide = m_pHeaderDlg->IsWindowVisible();
		m_pHeaderDlg->ShowWindow(bHide ? SW_HIDE : SW_SHOWNA);
		m_cToggleInfo.SetWindowText(bHide ? _T(">>") : _T("<<"));
		CRect r;
		GetWindowRect(&r);
		if (bHide)
			r.right -= m_nTabPosX;
		else
			r.right += m_nTabPosX;
		SetWindowPos(NULL, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER | SWP_DRAWFRAME);
		ResizeControls();
	}
}

void CFileBxDlg::OnExitButton() 
{
	if (AfxMessageBox(IDS_EXIT_PROMPT, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		OnExit();
}

void CFileBxDlg::OnExit() 
{
	PostMessage(WM_QUIT, 0, 0);	
}

LRESULT CFileBxDlg::OnQuit(WPARAM, LPARAM) 
{
	PostMessage(WM_QUIT, 0, 0);	
	return 0;
}

void CFileBxDlg::OnOpen() 
{
	ShowWindow(SW_RESTORE);
	::SetForegroundWindow( m_hWnd );	
}

void CFileBxDlg::OnDestroy() 
{
	OnTrayIconMsg(1, 0);
	WinHelp(0L, HELP_QUIT);
	int i;
	for (i = 0; i < m_nTabs; i++)
		m_pPage[i]->DestroyWindow();
	CDialog::OnDestroy();
}

BOOL CFileBxDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)     {
        AfxGetApp()->WinHelp( pHelpInfo->dwContextId,
                              HELP_CONTEXTPOPUP);
	    return TRUE;
    }

	return CDialog::OnHelpInfo(pHelpInfo);
}

#if 0
void MySendKeys(char *keys)
{
	int i, len = strlen(keys);
	int ch;
	INPUT in[2];
	BOOL bCapsWasPressed = FALSE;
	memset(&in[0], 0, sizeof(in));
	in[0].type = in[1].type = INPUT_KEYBOARD;
	in[1].ki.dwFlags = KEYEVENTF_KEYUP;

	if (GetKeyState(VK_CAPITAL) & 1)
	{
		bCapsWasPressed = TRUE;
		in[0].ki.wVk = in[1].ki.wVk = VK_CAPITAL;
		SendInput(2, in, sizeof(INPUT));
	}
	for (i = 0; i < len; i++)
	{
		ch = keys[i];
		int chu = ch;
		if (ch >= 'A' && ch <= 'Z') {
			in[0].ki.wVk = VK_SHIFT;
			SendInput(1, in, sizeof(INPUT));
		} else if (ch >= 'a' && ch <= 'z')
			chu -= ('a' - 'A');
		in[0].ki.wVk = in[1].ki.wVk = chu;
		SendInput(2, in, sizeof(INPUT));

		if (ch >= 'A' && ch <= 'Z') {
			in[1].ki.wVk = VK_SHIFT;
			SendInput(1, &in[1], sizeof(INPUT));
		}
	}
	if (bCapsWasPressed)
	{
		in[0].ki.wVk = in[1].ki.wVk = VK_CAPITAL;
		SendInput(2, in, sizeof(INPUT));
	}
}
#endif

BOOL CFileBxDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (wParam == 0 && lParam == 0)
	{ // Debug string message
		int nlines = m_pFavDlg->m_cDbgMsg.GetLineCount();
		int npos = m_pFavDlg->m_cDbgMsg.LineIndex(nlines+1);
		m_pFavDlg->m_cDbgMsg.SetSel(npos-1, npos);
		if (pShData)
			m_pFavDlg->m_cDbgMsg.ReplaceSel(pShData->buf);
		return FALSE;
	}
	else if (wParam == ID_EDIT_FV) 
	{
		if (lParam > 0 && lParam <= m_nTabs) {
			ShowWindow(SW_RESTORE);
			::SetForegroundWindow( m_hWnd );
			m_cTab.SetCurSel(lParam - 1);
			OnSelchangeTab1(NULL, NULL);
		} else if (lParam == 9) { 
			// Add/Remove folder message, with text in pShData->buf
			if (strstr(pShData->buf, pShData->szAdd) == pShData->buf) {
				// Add item
				TCHAR *pcTarget = pShData->buf + strlen(pShData->buf) + 1;
				TCHAR *pcDesc = strstr(pShData->buf, _T(": \""));
				TCHAR szDesc[MAX_PATH + 20];
				if (pcDesc) {
					pcDesc += 3; 
					TCHAR *pce = pcDesc + strlen(pcDesc) - 1;
					if (*pce == '\"')
						*pce = 0;
				} else {
					GetDirName(pcTarget, szDesc);
					pcDesc = szDesc;
				}
				m_pFavDlg->AddFavorite(pcDesc, pcTarget);
			} else {
				// Remove item
				TCHAR *pc = strstr(pShData->buf, _T(": \"")) + 3;
				TCHAR *pce = pc + strlen(pc) - 1;
				*pce = 0;
				m_pFavDlg->DeleteFavorite(pc);
			}
		}

		return FALSE;

	} else if (wParam == IDCANCEL) {
		if (GetFocus())
			GetFocus()->SendMessage(WM_KEYDOWN, VK_ESCAPE, 0);
		return FALSE;
	} else if (wParam == IDOK) {
		if (GetFocus())
			GetFocus()->SendMessage(WM_KEYDOWN, VK_RETURN, 0);
		return FALSE;
	} else if (wParam == IDC_FILLRECENT) {
		FillRecentFoldersBuffer();
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

