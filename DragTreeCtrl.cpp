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

// DragTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "FileBX.h"
#include "FavoritesDlg.h"
#include "SettingsDlg.h"
#include "KeySetDlg.h"
#include "OptionsDlg.h"
#include "ExceptionsDlg.h"
#include "FileBxDlg.h"
#include "DragTreeCtrl.h"
#include "DFH/DFH_shared.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TREE_SCROLL_TIMER 101

#define WM_HANDLE_DRAG WM_USER + 101
#define WM_REFRESH_TREE WM_USER + 102

static WORD cf_ShellIDL = 0;

/////////////////////////////////////////////////////////////////////////////
// COleDropSourceDF

COleDropSourceDF::COleDropSourceDF()
{
}

/////////////////////////////////////////////////////////////////////////////
// COleDropSourceDF overrides

// This function will control the movement and visibility of the 
// drag image.  Basically, it will hide the image when it is not 
// over the tree and move the image when it is.

SCODE COleDropSourceDF::GiveFeedback(DROPEFFECT /*dropEffect*/)
{
	ASSERT_VALID(this);
	ASSERT(NULL != m_pTree);

	POINT point;
	GetCursorPos(&point);

	//hides image when not over a view
	CRect rect;
	m_pTree->GetWindowRect(rect);

	// when you drag slowly across the window frame you will see 
	// that this adjustment is necessary.

	rect.DeflateRect(1, 1, 0, 1);

	if(rect.PtInRect(point))
	{
		// we are over the tree - if the image is hidden, show it 
		if(m_pTree->m_bImageHidden)
		{
			m_pTree->m_bImageHidden = FALSE;
			CImageList::DragEnter(m_pTree->GetDesktopWindow(), point);
		}

		m_pTree->m_pDrImList->DragMove(point);
	}
	else
	{
		if(!m_pTree->m_bImageHidden)
		{
			m_pTree->m_bImageHidden = TRUE;
			m_pTree->m_pDrImList->DragLeave(m_pTree->GetDesktopWindow());
		}
	}

	// don't change the cursor until drag is officially started
	return m_bDragStarted ? DRAGDROP_S_USEDEFAULTCURSORS : S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// CTreeDropTarget

CTreeDropTarget::CTreeDropTarget()
{
	if (!cf_ShellIDL)
		cf_ShellIDL = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
	m_ImageFormat = CF_HDROP;
}

CTreeDropTarget::~CTreeDropTarget()
{
}

DROPEFFECT CTreeDropTarget::OnDragEnter(CWnd* pWnd, COleDataObject* 
                           pDataObject, DWORD dwKeyState, CPoint point )
{   
	// Can we use this object?
	Msg(_T("Drag Enter"));

	m_dropEffectCurrent = DROPEFFECT_NONE;
	CDragTreeCtrl *pTree = (CDragTreeCtrl *) pWnd;

	if (pTree->m_hDragItem)
		m_dropEffectCurrent = DROPEFFECT_MOVE;

	else {
		if (pDataObject->IsDataAvailable(cf_ShellIDL)) {
			HGLOBAL hgData = (HGLOBAL) pDataObject->GetGlobalData(cf_ShellIDL);
			if (hgData) {
				CIDA *pCida = (CIDA *) GlobalLock(hgData);
				Msg(_T("CFSTR_SHELLIDLIST, cidl = %d"), pCida->cidl);
				for (int i = 1; i <= (int) pCida->cidl; i++) {
					ITEMIDLIST *pItemIDL = (ITEMIDLIST *)((BYTE *) pCida + pCida->aoffset[i]);
					TCHAR buf[MAX_PATH + 20];
					if (SHGetPathFromIDList(pItemIDL, buf)) {
						m_dropEffectCurrent = DROPEFFECT_LINK;
					}
				}

				GlobalUnlock(hgData);
			}
		}

		if (m_dropEffectCurrent == DROPEFFECT_NONE && 
			pDataObject->IsDataAvailable(m_ImageFormat))	
		{
			HDROP hDropInfo = (HDROP) pDataObject->GetGlobalData(m_ImageFormat);
			TCHAR szFile[MAX_PATH + 20], szDesc[MAX_PATH + 20];
			WIN32_FIND_DATA wfd;

			szFile[0] = 0;
			DragQueryFile(hDropInfo, 0, szFile, MAX_PATH);

			TCHAR *pc = szFile + strlen(szFile) - 5;

			if (strstr(pc, _T(".FX")) == pc && isxdigit(*(pc+3)) && isxdigit(*(pc+4))) {
				m_dropEffectCurrent = DROPEFFECT_MOVE;
			} else if (ResolveShortCut(NULL, szFile, wfd, NULL, szDesc) >= 0) {
				m_dropEffectCurrent = (strstr(szDesc, _T("HyperionicsDF")) == szDesc ?
					DROPEFFECT_MOVE : DROPEFFECT_LINK);
			} else
				m_dropEffectCurrent = DROPEFFECT_LINK;
		}
	} 
	
	if (m_dropEffectCurrent != DROPEFFECT_NONE)
		pTree->SetTimer(TREE_SCROLL_TIMER, 200, NULL);

	return m_dropEffectCurrent;
} 


void CTreeDropTarget::OnDragLeave(CWnd* pWnd)
{
	CDragTreeCtrl *pTree = (CDragTreeCtrl *) pWnd;
	pTree->KillTimer(TREE_SCROLL_TIMER);

	CImageList::DragLeave(NULL);
	pTree->DrawInsertLine(); // Erase the old line
	CPoint point;
	::GetCursorPos(&point);
	CImageList::DragEnter(pTree->GetDesktopWindow(), point);
	pTree->m_htiTarget = NULL;
	COleDropTarget::OnDragLeave(pWnd);
}


DROPEFFECT CTreeDropTarget::OnDragOver(CWnd* pWnd, COleDataObject* 
           pDataObject, DWORD dwKeyState, CPoint point )
{
	if (m_dropEffectCurrent == DROPEFFECT_NONE)
		return m_dropEffectCurrent;

	CDragTreeCtrl *pTree = (CDragTreeCtrl *) pWnd;
	pTree->OnDragOver(pDataObject, dwKeyState, point);

	return m_dropEffectCurrent;
}


BOOL CTreeDropTarget::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, 
                 DROPEFFECT dropEffect, CPoint point )
{
	CDragTreeCtrl *pTree = (CDragTreeCtrl *) pWnd;
	pTree->KillTimer(TREE_SCROLL_TIMER);// must be repeated here for items
										// dragged from outside

	const int MAX_NAMES = 64;
	int iFile, nFiles = 0;
	CString sNames[MAX_NAMES];
	TCHAR szFile[MAX_PATH + 20];

	HGLOBAL hgData = pDataObject->GetGlobalData(m_ImageFormat);
	if (hgData) {
		nFiles = DragQueryFile((HDROP) hgData, 0xFFFFFFFF, szFile, MAX_PATH);
		if (nFiles > MAX_NAMES)
			nFiles = MAX_NAMES;
		for (iFile = 0; iFile < nFiles; iFile++) {
			DragQueryFile((HDROP) hgData, iFile, szFile, MAX_PATH);
			sNames[iFile] = szFile;
		}

	} else if (hgData = pDataObject->GetGlobalData(cf_ShellIDL)) {
		CIDA *pCida = (CIDA *) GlobalLock(hgData);
		Msg(_T("CFSTR_SHELLIDLIST, cidl = %d"), pCida->cidl);
		nFiles = 0;
		for (int i = 1; i <= (int) pCida->cidl; i++) {
			ITEMIDLIST *pItemIDL = (ITEMIDLIST *)((BYTE *) pCida + pCida->aoffset[i]);
			if (SHGetPathFromIDList(pItemIDL, szFile)) {
				sNames[nFiles++] = szFile;
			}
		}

		GlobalUnlock(hgData);
	}

	if (nFiles) {
		pTree->OnDropFiles(sNames, nFiles);
		pTree->Invalidate();

	}

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CDragTreeCtrl

CDragTreeCtrl::CDragTreeCtrl()
{
	x_old_line = 0;
	y_old_line = -10001; // No line.

	m_htiTarget = NULL;	// handle to target item 
	m_hDragItem = NULL;
	m_iInsert = -1;		// 0 before, 1 on, 2 after
	m_pszFolder[0] = 0;
	m_pDrImList = NULL;
	m_bImageHidden = TRUE;
	m_bDropRegistered = FALSE;
	m_hChange = NULL;

	m_OleDropSource.m_pTree = this;
	m_hResMutex = CreateMutex(NULL, FALSE, _T("R") HPNAME);
}

CDragTreeCtrl::~CDragTreeCtrl()
{
	CloseHandle(m_hResMutex);
}


BEGIN_MESSAGE_MAP(CDragTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CDragTreeCtrl)
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_HANDLE_DRAG, OnHandleDrag)
	ON_MESSAGE(WM_REFRESH_TREE, OnRefreshMessage)
END_MESSAGE_MAP()


CString CDragTreeCtrl::GetFileName(HTREEITEM hItem, TCHAR *pszPath /* = NULL */)
// returns full path + file/folder name + ext. of the file/folder 
// corresponding to hItem (link file, text file or sub-folder).
// pszPath is the the full folder path of the folder where the given 
// link, text file or sub-folder resides.
{
	TCHAR szPath[MAX_PATH + 20];
	TCHAR s[20];
	CString strItem;
	strcpy(szPath, m_pszFolder);
	strcat(szPath, _T("\\"));
	if (!hItem || hItem == TVI_ROOT || hItem == TVI_FIRST || hItem == TVI_LAST) {
		strItem = szPath;
		if (pszPath)
			strcpy(pszPath, szPath);
		return strItem;
	}
	HTREEITEM hParentItem = GetParentItem(hItem);

	if (hParentItem && hParentItem != TVI_ROOT) {
		HTREEITEM hi[256];
		int n = 0, m;

		hi[n++] = hParentItem;
		while (hi[n] = GetParentItem(hi[n-1]))
			n++;
		for (--n; n >= 0; n--) {
			strItem = GetItemText(hi[n]);
			strItem = EncodeFileName(strItem);
			m = GetItemData(hi[n]);
			wsprintf(s, _T(".FX%02x"), m);
			strcat(szPath, strItem + s);
			strcat(szPath, _T("\\"));
		}
	}
	strItem = GetItemText(hItem);
	strItem = EncodeFileName(strItem);

	int nImage, n;
	GetItemImage(hItem, nImage, n);
	if (nImage >= FOLDER_IMAGE) {
		n = GetItemData(hItem);
		wsprintf(s, _T(".FX%02x"), n);
		strItem = szPath + strItem + s;
	} else
		strItem = szPath + strItem + _T(".lnk");
	if (pszPath)
		strcpy(pszPath, szPath);
	return strItem;
}


void CDragTreeCtrl::RenumberItems(HTREEITEM hParentItem)
{
	HTREEITEM hItem;
	TCHAR szPath[MAX_PATH + 20];

	hItem = GetNextItem(hParentItem, TVGN_CHILD);
	GetFileName(hItem, szPath); // ignore ret. name, need only path
	int nItemNo = 0;
	EnableFolderWatch(FALSE);
	while (hItem) {
		int nImage, n;
		GetItemImage (hItem, nImage, n);
		CString strItem = GetItemText(hItem);
		strItem = EncodeFileName(strItem);
		strItem = szPath + strItem;

		if (nImage >= FOLDER_IMAGE) {
			TCHAR s[20];
			wsprintf(s, _T(".FX%02x"), nItemNo);
			CString sNewFileName = strItem + s;
			n = GetItemData(hItem);
			wsprintf(s, _T(".FX%02x"), n);
			CString sOldFileName = strItem + s;
			MoveFile(sOldFileName, sNewFileName);
		} else {
			strItem += _T(".lnk");
			SetItemNumber(m_hWnd, strItem, nItemNo);
		}

		SetItemData(hItem, (DWORD) nItemNo);
		hItem = GetNextSiblingItem(hItem);
		nItemNo++;
	}
	EnableFolderWatch(TRUE);
	if (pShData)
		CreateMenuResource((BYTE *) pShData->pcFavorites, sizeof(pShData->pcFavorites));
}


/////////////////////////////////////////////////////////////////////////////
// CDragTreeCtrl message handlers

void CDragTreeCtrl::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent) {
	case TREE_SCROLL_TIMER:
		static CPoint cpLast(0, 0);
		static int n = 0;

		CPoint pt, cpt;
		CRect rct;
		::GetCursorPos(&pt);
		if (cpLast == pt)
			n++;
		else {
			n = 0;
			cpLast = pt;
		}
		cpt = pt;
		ScreenToClient(&cpt);
		GetClientRect(&rct);
		if (rct.PtInRect(cpt)) {
			m_pDrImList->DragLeave(NULL);
			int h = GetItemHeight();
			
			if (cpt.y >= 0 && cpt.y < h) {
				DrawInsertLine();
				SendMessage(WM_VSCROLL, SB_LINEUP, 0);
			} else if (cpt.y <= rct.bottom && cpt.y > rct.bottom - h) {
				DrawInsertLine();
				SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			} 
		
			if (cpt.x >=0 && cpt.x < h) {
				DrawInsertLine();
				SendMessage(WM_HSCROLL, SB_LINEUP, 0);
			} else if (cpt.x <= rct.right && cpt.x > rct.right - h) {
				DrawInsertLine();
				SendMessage(WM_HSCROLL, SB_LINEDOWN, 0);
			}

			if (n >= 5) {
				UINT flags = 0;
				HTREEITEM hItem;
				hItem = HitTest(cpt, &flags);
				if (hItem && hItem != m_hDragItem && m_iInsert == 1) {
					Expand(hItem, TVE_EXPAND);
				}
				n = 0;
			}
			m_pDrImList->DragEnter(NULL, pt);
		}
		break;
	}
	
	CTreeCtrl::OnTimer(nIDEvent);
}


void CDragTreeCtrl::DrawInsertLine(int x, int y)
{
	if (y <= -10000) { // erase old line
		if (y_old_line <= -10000) // no old line
			return;
		y = y_old_line;
		y_old_line = -10000;
		// proceed below to delete it.
	} else
		y_old_line = y;
	if (x <= -10000)
		x = x_old_line;
	else
		x_old_line = x;

	CRect rct;
	GetClientRect(&rct);
	rct.left += x;
	ClientToScreen(&rct);
	if (!rct.PtInRect(CPoint(rct.left+1, y))) {
		y_old_line = -10000;
		return;
	}
	HDC hdc = ::GetDC(NULL);
	HPEN hpen = ::CreatePen(PS_SOLID, 2, 0xffffff);
	hpen = (HPEN) ::SelectObject(hdc, hpen);
	::SetROP2(hdc, R2_XORPEN);
	int h = GetItemHeight()/2;

	::MoveToEx(hdc, rct.left, y-h/2, NULL);
	::LineTo(hdc, rct.left+h, y);
	::LineTo(hdc, rct.right-h, y);
	::LineTo(hdc, rct.right, y-h/2);
	::MoveToEx(hdc, rct.left, y+h/2, NULL);
	::LineTo(hdc, rct.left+h, y);
	::MoveToEx(hdc, rct.right-h+1, y+1, NULL);
	::LineTo(hdc, rct.right, y+h/2);
	
	hpen = (HPEN) ::SelectObject(hdc, hpen);
	::DeleteObject(hpen);
	::ReleaseDC(NULL, hdc);
}


LRESULT CDragTreeCtrl::OnRefreshMessage(WPARAM, LPARAM)
{
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_REFRESH_TREE, WM_REFRESH_TREE, PM_REMOVE));
	RefreshTree();
	return 0;
}


static int CALLBACK MyCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	if (lParam1 < lParam2)
		return -1;
	else if (lParam1 == lParam2)
		return 0;
	return 1;
}


UINT CDragTreeCtrl::ThreadFunc(LPVOID pParam)
{
	CDragTreeCtrl *pTree = (CDragTreeCtrl *) pParam;

	if (!pTree->m_hChange)
		return 0;

    // Wait until a file change notification wakes this thread or
    // m_event becomes set indicating it's time for the thread to end.
    if (::WaitForSingleObject(pTree->m_hChange, INFINITE) == WAIT_OBJECT_0) { 
		if (pTree->m_hChange) {
			// tell the tree to refresh itself.
			Msg(_T("Got change notification!"));
			Sleep(500);
			::PostMessage (pTree->m_hWnd, WM_REFRESH_TREE, (WPARAM) 0, 0);
			// ::FindNextChangeNotification (pTree->m_hChange); no need! RefreshTree() will set it again!
		}
    }

	if (pTree->m_hChange) {
		HANDLE hChange = pTree->m_hChange;
		pTree->m_hChange = NULL;
		::FindCloseChangeNotification(hChange);
	}

    return 0;
}

void CDragTreeCtrl::EnableFolderWatch(BOOL bEnable)
{
	if (bEnable) {
		if (!m_hChange) {
			m_hChange = ::FindFirstChangeNotification(m_pszFolder, TRUE, 
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME);
			if (m_hChange && m_hChange != INVALID_HANDLE_VALUE) {
				CWinThread* pThread = AfxBeginThread (ThreadFunc, (LPVOID) this,
					THREAD_PRIORITY_BELOW_NORMAL);
				pThread->m_bAutoDelete = TRUE;
				Msg(_T("Started thread 0x%x for change notification handle 0x%x"), pThread->m_nThreadID, m_hChange);
			} else {
				m_hChange = NULL;
				Msg(_T("Failed in FindFirstChangeNotification()"));
			}
		} else
			Msg(_T("m_hChange was non-NULL: 0x%x"), m_hChange);

	} else if (m_hChange) {
		HANDLE hChange = m_hChange;
		m_hChange = NULL;
		Msg(_T("Closing change notification handle 0x%x"), hChange);
		::FindCloseChangeNotification(hChange);
		// above also signals the wait function.
		::Sleep(200);
	}
}


void CDragTreeCtrl::RefreshTree(HTREEITEM hParent /* = TVI_ROOT*/, 
								TCHAR *pszFolder /* = NULL */, 
								int iResolveLinks /* = FALSE*/)
{
	WIN32_FIND_DATA wfd, wfd2;
	TCHAR buf[MAX_PATH + 16], fname[MAX_PATH], szPath[MAX_PATH], szDesc[MAX_PATH];
	TCHAR *pc;
	int i, nImage, n;
	HTREEITEM hItem, hNextItem;
	HANDLE hff;

	if (!hParent || hParent == TVI_ROOT) {
		EnableFolderWatch(FALSE);
		Msg(_T("Refreshing root"));
    }
	
	if (!pszFolder)
		pszFolder = m_pszFolder;

	// Now also go through all sub-folders, and if any of them
	// does not have a .FXxx extension, add .FXff.
	strcpy(buf, pszFolder);
	strcat(buf, _T("\\*"));
	hff = FindFirstFile(buf, &wfd);
	if (hff != INVALID_HANDLE_VALUE) {
		do {
			if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
				strcmp(wfd.cFileName, _T(".")) && strcmp(wfd.cFileName, _T("..")))
			{
				strcpy(fname, wfd.cFileName);
				strcpy(buf, pszFolder);
				strcat(buf, _T("\\"));
				strcat(buf, fname);
				strcpy(szPath, buf);
				pc = buf + strlen(buf) - 5;
				if (strstr(pc, _T(".FX")) != pc) {
					pc = strchr(pc, '.');
					if (pc)
						*pc = 0;
					else
						pc = buf + strlen(buf);
					strcat(pc, _T(".FXff"));
					i = 1;
					while (!MoveFile(szPath, buf)) {
						*pc = 0;
						wsprintf(pc, _T(" %d.FXff"), i++);
					}
				}
			}
		} while (FindNextFile(hff, &wfd));

		FindClose(hff);
	}

	// Check all the items in the current branch - if corresponding
	// *.lnk or .FXxx does not exist, delete it.
	hItem = GetChildItem(hParent);
	while (hItem) {
		hNextItem = GetNextItem(hItem, TVGN_NEXT);
		GetItemImage(hItem, nImage, n);
		n = GetItemData(hItem);
		CString sItem = EncodeFileName(GetItemText(hItem));

		if (nImage < FOLDER_IMAGE)
			wsprintf(buf, _T("%s\\%s.lnk"), pszFolder, sItem);
		else
			wsprintf(buf, _T("%s\\%s.FX%02x"), pszFolder, sItem, n);
		if (_access(buf, 0) < 0) {
			DeleteItem(hItem);
		}
		hItem = hNextItem;
	}

	// Now go through all *.lnk to insert missing items into the 
	// tree control:
	strcpy(buf, pszFolder);
	strcat(buf, _T("\\*.lnk"));
	hff = FindFirstFile(buf, &wfd);
	if (hff != INVALID_HANDLE_VALUE) {
		do {
			strcpy(fname, wfd.cFileName);
			strcpy(buf, pszFolder);
			strcat(buf, _T("\\"));
			strcat(buf, fname);
			pc = fname + strlen(fname) - 4;
			if (*pc == '.')
				*pc = 0;
			// First, search the tree branch if that item is already there:
			hItem = GetChildItem(hParent);
			while (hItem) {
				CString s = EncodeFileName(GetItemText(hItem));
				if (strcmp(s, fname) == 0)
					break;
				hItem = GetNextItem(hItem, TVGN_NEXT);
			}

			if (ResolveShortCut(m_hWnd, buf, wfd2, szPath, szDesc, iResolveLinks) == 0)
				//_access(szPath, 0) == 0) 
			{
				BOOL bExists = (iResolveLinks < 2) || (_access(szPath, 0) == 0);
				DWORD nSeq = (DWORD) atoi(szDesc + strlen(_T("HyperionicsDF ")));

				if (bExists) {
					if (!(wfd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						if (!hItem)
							hItem = InsertItem(DecodeFileName(fname), DOC_IMAGE, DOC_IMAGE, hParent);
					} else if (!hItem) {
						hItem = InsertItem(DecodeFileName(fname), FAV_FOLDER_IMAGE, FAV_FOLDER_IMAGE, hParent);
					}
					SetItemData(hItem, nSeq);

				} else {
					// this link can not be resolved any more, delete link and hItem.
					wsprintf(buf, _T("%s\\%s.lnk"), pszFolder, fname);
					DeleteFile(buf);
					if (hItem)
						DeleteItem(hItem);
					if (iResolveLinks == 2) {
						// convert broken links to text items
						wsprintf(buf, _T("%s\\%s.FX%02x"), pszFolder, fname, nSeq);
						FILE *fp = fopen(buf, _T("w"));
						if (fp) {
							fputs(szPath, fp);
							fclose(fp);
						}
					}
				}
			}

		} while (FindNextFile(hff, &wfd));

		FindClose(hff);
	}

	// Now go through all *.FXxx to insert missing items into the 
	// tree control:
	strcpy(buf, pszFolder);
	strcat(buf, _T("\\*.FX??"));
	hff = FindFirstFile(buf, &wfd);
	if (hff != INVALID_HANDLE_VALUE) {
		do {
			strcpy(fname, wfd.cFileName);
			strcpy(buf, pszFolder);
			strcat(buf, _T("\\"));
			strcat(buf, fname);
			pc = fname + strlen(fname) - 5;
			if (strstr(pc, _T(".FX")) != pc || !isxdigit(*(pc+3)) || !isxdigit(*(pc+4))) {
				DeleteFile(buf);
				continue;
			}
			int n_ext;
			sscanf(pc+3, _T("%x"), &n_ext);
			*pc = 0;
			// First, search the tree branch if that item is already there:
			hItem = GetChildItem(hParent);
			while (hItem) {
				CString s = EncodeFileName(GetItemText(hItem));
				if (strcmp(s, fname) == 0) {
					n = GetItemData(hItem);
					if (n == n_ext)
						break;
				}
				hItem = GetNextItem(hItem, TVGN_NEXT);
			}

			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				if (!hItem) {
					int n = TEXT_IMAGE;
					if (strstr(fname, _T("---")) == fname)
						n = SEP_IMAGE;
					hItem = InsertItem(DecodeFileName(fname), n, n, hParent);
				}
			} else {
				if (!hItem) 
					hItem = InsertItem(DecodeFileName(fname), FOLDER_IMAGE, FOLDER_IMAGE, hParent);
				RefreshTree(hItem, buf, iResolveLinks);
			}
			SetItemData(hItem, (DWORD) n_ext);

		} while (FindNextFile(hff, &wfd));

		FindClose(hff);
	}

	if (!hParent || hParent == TVI_ROOT) {
		EnableFolderWatch(TRUE);
    }

	TVSORTCB tvs;
	// Sort the tree control's items using my callback procedure.
	tvs.hParent = hParent;
	tvs.lpfnCompare = MyCompareProc;
	tvs.lParam = (LPARAM) this;

	SortChildrenCB(&tvs);
	if (!m_bDropRegistered) {
		m_bDropRegistered = TRUE;
		BOOL bRet = m_OleDropTarget.Register(this);
	}
	if (pShData)
		CreateMenuResource((BYTE *) pShData->pcFavorites, sizeof(pShData->pcFavorites));
}


void CDragTreeCtrl::MoveTree(HTREEITEM hDest, HTREEITEM hSrc, HTREEITEM hInsertAfter)
{
	BOOL bMoveBranch = TRUE;

	if (hDest != GetParentItem(hSrc)) {
		TCHAR szPath[MAX_PATH + 20];
		// also move the folder itself, if source is a link to our sub-menu folder...
		int nImage, nSelectedImage;
		GetItemImage (hSrc, nImage, nSelectedImage);

		CString sSrcName = GetFileName(hSrc);
		if (hDest) {
			strcpy(szPath, GetFileName(hDest));
		} else
			strcpy(szPath, m_pszFolder);
		const TCHAR *pc = (const TCHAR *) sSrcName + strlen(sSrcName) - 1;
		while (pc > (const TCHAR *) sSrcName && *pc != '\\')
			pc--;
		CString sSrcNewName(szPath);
		sSrcNewName += pc;
		
		EnableFolderWatch(FALSE);
		if (!MoveFile(sSrcName, sSrcNewName)) {
			AfxMessageBox(IDS_ERR_MOVE, MB_OK | MB_ICONSTOP);
			bMoveBranch = FALSE;
		}
		EnableFolderWatch(TRUE);
	}

	if (bMoveBranch) {
		CopyTree (hDest, hSrc, hInsertAfter);
		DeleteItem (hSrc);
	}
}

void CDragTreeCtrl::CopyTree(HTREEITEM hDest, HTREEITEM hSrc, HTREEITEM hInsertAfter)
{
	//
	// Get the attributes of item to be copied.
	//
	int nImage, n;
	GetItemImage (hSrc, nImage, n);
	CString string = GetItemText (hSrc);

	//
	// Create an exact copy of the item at the destination.
	//
	HTREEITEM hNewItem = InsertItem (string, nImage, n, hDest, hInsertAfter);
	n = GetItemData(hSrc);
	SetItemData(hNewItem, (DWORD) n);

	//
	// If the item has subitems, copy them, too.
	//
	if (ItemHasChildren (hSrc))
		CopyChildren (hNewItem, hSrc);

	//
	// Select the newly added item.
	//
	SelectItem (hNewItem);
}

void CDragTreeCtrl::CopyChildren(HTREEITEM hDest, HTREEITEM hSrc)
{
	//
	// Get the first subitem.
	//
	HTREEITEM hItem = GetChildItem (hSrc);
	ASSERT (hItem != NULL);
	CString string;
	HTREEITEM hNewItem;
	int nImage, n;

	do {
		//
		// Create a copy of it at the destination.
		//
		GetItemImage (hItem, nImage, n);
		string = GetItemText (hItem);
		hNewItem = InsertItem (string, nImage, n, hDest);
		n = GetItemData(hItem);
		SetItemData(hNewItem, (DWORD) n);
		//
		// If the subitem has subitems, copy them, too.
		//
		if (ItemHasChildren (hItem))
			CopyChildren (hNewItem, hItem);
	} while (hItem = GetNextSiblingItem (hItem));

}


/////////////////////////////////////////////////////////////////////////////
// OnBeginDrag will be called in response to the TVN_BEGINDRAG message.
// This is the same place that a common control would normally handle 
// beginning a drag drop operation.  Basically what we do here is load 
// up a COleDataSource and post a message for the drag drop operation 
// to be handled.  Posting the message insures that the common control 
// can "normalize" before the Drag Drop operation is started.

void CDragTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	int nImage, nSelectedImage;
	CRect rct, rctWin;
	m_hDragItem = pNMTreeView->itemNew.hItem; // item being dragged
	Expand(m_hDragItem, TVE_COLLAPSE);
	SelectItem(NULL);
	UpdateWindow();

	// Load a CF_HDROP to the data object 
	TCHAR *names[2], fname[MAX_PATH + 20]; 
	CString cs = GetFileName(m_hDragItem);

	GetItemImage (m_hDragItem, nImage, nSelectedImage);
	strcpy(fname, cs);
	if (nImage == FOLDER_IMAGE) {
		// We want to drag the folder itself, not the link...
		TCHAR *pc = fname + strlen(fname) - 4;
		if (_stricmp(pc, _T(".lnk")) == 0)
			*pc = 0;
	}

	names[0] = fname;
    HGLOBAL hgCF_HDROPData = CreateHDrop(1, names);   

	// allocate our data object
	COleDataSource* pDataSource;
	pDataSource = new COleDataSource;
	pDataSource->CacheGlobalData( CF_HDROP, hgCF_HDROPData );

	PostMessage(WM_HANDLE_DRAG, (UINT)pDataSource, 0);

	*pResult = 0;
}


/////////////////////////////////////////////////////////////////////////////
// OnHandleDrag sets up the drag image, calls DoDragDrop and cleans up 
// after the drag drop operation finishes.

LRESULT CDragTreeCtrl::OnHandleDrag(WPARAM wParam, LPARAM /* lParam = 0*/)
{
	POINT point, pointClient;
	COleDataSource* pDataSource = (COleDataSource *) wParam;
	CRect rct;
	GetCursorPos(&point);

	pointClient = point;
	ScreenToClient(&pointClient);

	m_pDrImList = CreateDragImage(m_hDragItem);
	// Get the bounding rectangle of the item being dragged. 
	GetItemRect(m_hDragItem, &rct, TRUE);

    // Get the heading level and the amount that the child items are 
    // indented. 
    DWORD dwLevel = 0; 
	HTREEITEM hParentItem = GetParentItem(m_hDragItem);
	for (; hParentItem; hParentItem = GetParentItem(hParentItem))
		dwLevel++;
    DWORD dwIndent = GetIndent();
	rct.OffsetRect(CPoint(dwLevel*dwIndent, 0));
	ClientToScreen(&rct);

	// changes the cursor to the drag image (DragMove() is still required 
    m_pDrImList->BeginDrag(0, CPoint(0, rct.Height()/2)); 
	m_pDrImList->DragEnter(GetDesktopWindow(), point);

	if(DROPEFFECT_MOVE == (pDataSource)->DoDragDrop
			( DROPEFFECT_MOVE | DROPEFFECT_LINK, NULL, 
			&m_OleDropSource))
	{
		; // No need to do anything here, folder watch thread will fix us.
	}

	delete pDataSource;

	m_pDrImList->DragLeave(GetDesktopWindow());
	m_pDrImList->EndDrag();
	KillTimer(TREE_SCROLL_TIMER);
	m_hDragItem = NULL;

	delete m_pDrImList;
	m_pDrImList = NULL;

	return 0;
}


void CDragTreeCtrl::OnDragOver(COleDataObject* /*pDataObject*/, DWORD /*dwKeyState*/, CPoint point)
{
    TVHITTESTINFO tvht;  // hit test information 
 
	CPoint pt = point;
	ClientToScreen(&pt);
    // Drag the item to the current position of the mouse cursor. 
	// if (m_pDrImList)
	//    m_pDrImList->DragMove(pt); 

    // Find out if the cursor is on the item. If it is, highlight 
    // the item as a drop target. 
	ScreenToClient(&pt);
    tvht.pt.x = pt.x; 
    tvht.pt.y = pt.y; 
	m_htiTarget = HitTest(&tvht);
	BOOL bLastItem = FALSE;
	if (!m_htiTarget && pt.y > 4) {
		HTREEITEM hItem = GetRootItem();;
		while (hItem = GetNextItem(hItem, TVGN_NEXT))
			m_htiTarget = hItem;
		bLastItem = TRUE;
	}
	ClientToScreen(&pt);

    if (m_htiTarget != NULL && m_htiTarget != m_hDragItem) { 
		CRect rct;
		GetItemRect(m_htiTarget, &rct, TRUE);
		int n, m;
		GetItemImage(m_htiTarget, n, m);
		int x = rct.left - 16;
		int h = (n == FOLDER_IMAGE ? rct.Height()/4 : rct.Height()/2);

		ClientToScreen(&rct);
		if (abs(rct.top - pt.y) <= h) {
			m_iInsert = 0; // before
			if (y_old_line != rct.top) {
				CImageList::DragLeave(NULL);
				DrawInsertLine(); // Erase the old line
		        SelectDropTarget(NULL);
				DrawInsertLine(x, rct.top);
				CImageList::DragEnter(NULL, pt);
			}
		} else if (abs(rct.bottom - pt.y) < h || bLastItem) {
			m_iInsert = 2;	// after
			if (y_old_line != rct.bottom) {
				CImageList::DragLeave(NULL);
				DrawInsertLine(); // Erase the old line
		        SelectDropTarget(NULL); 
				DrawInsertLine(x, rct.bottom);
				CImageList::DragEnter(NULL, pt);
			}
		} else if (n == FOLDER_IMAGE) {
				m_iInsert = 1; // on
				CImageList::DragLeave(NULL);
				DrawInsertLine(); // Erase the old line
				SelectDropTarget(m_htiTarget); 
				CImageList::DragEnter(NULL, pt);
		} else
			m_htiTarget = m_hDragItem; // will be ignored
    } else {
		m_htiTarget = m_hDragItem; // will be ignored
	}

}


void CDragTreeCtrl::OnDropFiles(CString sNames[], int nFiles) 
{
	CImageList::DragLeave(NULL);
	DrawInsertLine();
	SelectDropTarget(NULL);
	if (m_hDragItem  && (!m_htiTarget|| m_htiTarget == m_hDragItem))
		return;

	HTREEITEM hInsertAfter = TVI_LAST, hParentItem;
	if (m_iInsert == 1) { // On
		hParentItem = m_htiTarget;
	} else if (m_iInsert == 2) { // Below
		hParentItem = GetParentItem(m_htiTarget);
		hInsertAfter = m_htiTarget;
	} else { // Above
		hParentItem = GetParentItem(m_htiTarget);
		if (!(hInsertAfter = GetPrevSiblingItem(m_htiTarget)))
			hInsertAfter = TVI_FIRST;
	}

	if (m_hDragItem) { // we were dragging our own item...
		MoveTree(hParentItem, m_hDragItem, hInsertAfter);
		RenumberItems(hParentItem);

	} else { // something else dropped on us
		int iFile, n;
		TCHAR szFile[MAX_PATH + 20], szTarget[MAX_PATH + 20], szPath[MAX_PATH + 20], szDesc[MAX_PATH+20];
		TCHAR fname[_MAX_FNAME];
		CString sLinkDir(m_pszFolder);
		WIN32_FIND_DATA wfd;

		if (hParentItem && hParentItem != TVI_ROOT) {
			sLinkDir = GetFileName(hParentItem);
		}
		EnableFolderWatch(FALSE);

		for (iFile = 0; iFile < nFiles; iFile++) {
			strcpy(szFile, sNames[iFile]);
			int nExtNo = 0;

			TCHAR *pc = szFile + strlen(szFile) - 5;
			if (strstr(pc, _T(".FX")) == pc && isxdigit(*(pc+3)) && isxdigit(*(pc+4))) {
				TCHAR ext[128];
				sscanf(pc+3, _T("%x"), &nExtNo);
				_splitpath(szFile, NULL, NULL, fname, ext);
				wsprintf(szTarget, _T("%s\\%s%s"), sLinkDir, fname, ext);
				int i = 0;
				while (_access(szTarget, 0) == 0) {
					// this file already exists.
					wsprintf(szTarget, _T("%s\\%s %d.%s"), sLinkDir, fname, ++i, ext);
				}
				if (i)
					wsprintf(fname, _T("%s %d"), fname, i);
				
				if (IsFolder(szFile))
					n = FOLDER_IMAGE;
				else {
					n = (strstr(fname, _T("---")) == fname) ? SEP_IMAGE : TEXT_IMAGE;
				}

				if (!MoveFileOrFolder(szFile, szTarget)) {
					n = -1;
				}

			} else {
				if (ResolveShortCut(m_hWnd, szFile, wfd, szTarget, szDesc) < 0) {
					strcpy(szTarget, szFile);
				} else if (strstr(szDesc, _T("HyperionicsDF ")) == szDesc)
					DeleteFile(szFile);
				_splitpath(szFile, NULL, NULL, fname, NULL);
				wsprintf(szPath, _T("%s\\%s.lnk"), sLinkDir, fname);
				int i = 0;
				while (_access(szPath, 0) == 0) {
					// this file already exists.
					wsprintf(szPath, _T("%s\\%s %d.lnk"), sLinkDir, fname, ++i);
				}
				if (i)
					wsprintf(fname, _T("%s %d"), fname, i);
				CreateShortCut(szTarget, szPath);
				HANDLE hFind = ::FindFirstFile(szTarget, &wfd);
				n = DOC_IMAGE;
				if (hFind != INVALID_HANDLE_VALUE) {
					::FindClose(hFind);
					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						n = FAV_FOLDER_IMAGE;
				}
			}

			if (n >= 0)  {
				hInsertAfter = InsertItem(DecodeFileName(fname), n, n, hParentItem, hInsertAfter);
				SetItemData(hInsertAfter, nExtNo);
			}
		}
		
		RenumberItems(hParentItem);
		RefreshTree();
		Expand(hParentItem, TVE_EXPAND);
		SelectItem (hInsertAfter);
	}
}

typedef UINT (CALLBACK *DFH_ADDMENUITEM)(BYTE *MenuTemplate, const TCHAR *pcMenuString, WORD MenuID, 
				 BOOL IsPopup, BOOL LastItem);

DFH_ADDMENUITEM pDFH_AddMenuItem = NULL;

static char *pStore = NULL;
static int nStored = 0;

static void StoreData(char *pData, int nBytes) // keep char * here!!!
{
	static int nAllocated = 4096;
	if (!pStore)
		pStore = (char *) malloc(nAllocated);
	if (nStored + nBytes >= nAllocated) {
		nAllocated = nStored + nBytes + 4096;
		pStore = (char *) realloc(pStore, nAllocated);
	}
	memcpy(pStore + nStored, pData, nBytes);
	nStored += nBytes;
}

DWORD CDragTreeCtrl::CreateMenuResource(BYTE *pMenuBuffer, DWORD dwBufLen, 
								HTREEITEM hParent /* = TVI_ROOT */)
// Returns actual bytes used, 0 if failed (e.g. dwBufLen too small)
{
	HTREEITEM hItem, hChildItem, hNextItem;
	TCHAR buf[MAX_PATH + 20];
	int bytes_used = 0;

	if (hParent == TVI_ROOT) {

		if (!pDFH_AddMenuItem) {
			if (!hInstDLL)
				hInstDLL = LoadLibrary(_T("FileBXH.dll"));
			if (hInstDLL) {
				pDFH_AddMenuItem = (DFH_ADDMENUITEM) GetProcAddress(hInstDLL, "DFH_AddMenuItem"); 
			}
			if (!pDFH_AddMenuItem)
				return 0;
		}
		if (WaitForSingleObject(m_hResMutex, 1000) != WAIT_OBJECT_0) 
			return 0;
		memset(pMenuBuffer, 0, dwBufLen);
		// Fill up the MENUEX_TEMPLATE_HEADER structure.
		MENUEX_TEMPLATE_HEADER* mheader = (MENUEX_TEMPLATE_HEADER*) pMenuBuffer;
		mheader->wVersion = 1;
		mheader->wOffset = 4;
		mheader->dwHelpId = 0;
		bytes_used = sizeof(MENUEX_TEMPLATE_HEADER);

		nStored = 0;
		StoreData((char*) _T("0123456789"), 10*sizeof(TCHAR)); // necessary, reserving menu IDs 0 - 9
	} else if (!pDFH_AddMenuItem)
		return 0;


	int nItems = 0;
	hItem = GetChildItem(hParent);
	while (hItem) {
		strcpy(buf, GetItemText(hItem));
		hChildItem = 	GetChildItem(hItem);
		nItems++;

		hNextItem = GetNextItem(hItem, TVGN_NEXT);

		bytes_used += pDFH_AddMenuItem(pMenuBuffer + bytes_used, buf, 
			(WORD) nStored,
			hChildItem ? TRUE : FALSE, 
			(hParent == TVI_ROOT) || hNextItem ? FALSE : TRUE);
		if (hChildItem) {
			DWORD dwUsed = CreateMenuResource(pMenuBuffer + bytes_used, 
				dwBufLen - bytes_used, hItem);
			if (!dwUsed) {
				if (hParent == TVI_ROOT)
					ReleaseMutex(m_hResMutex);
				return 0;
			}
			bytes_used += dwUsed;

		} else {
			CString s = GetFileName(hItem);
			const TCHAR *pc = (const TCHAR *) s + strlen(s) - 5;
			*buf = 0;
			if (strstr(pc, _T(".FX")) == pc) {
				FILE *fp = fopen(s, _T("rb"));
				if (fp) {
					fgets(buf, MAX_PATH + 20, fp);
					fclose(fp);
					TCHAR *pc = buf + strlen(buf);
					while (pc > buf && *(pc-1) < ' ')
						pc--;
					*pc = 0;
				}
			} else {
				WIN32_FIND_DATA wfd;
				ResolveShortCut(m_hWnd, s, wfd, buf, NULL);
				if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					buf[strlen(buf) - 1] != '\\')
					strcat(buf, _T("\\"));
			}
			StoreData((char *)buf, ((UINT)strlen(buf) + 1)*sizeof(TCHAR));
		}
		hItem = hNextItem;
	}

	if (hParent == TVI_ROOT) {
		CString s;
		bytes_used += pDFH_AddMenuItem(pMenuBuffer + bytes_used, _T("---"), 
			(WORD) 0, FALSE, FALSE);

		s.LoadString(IDS_MAINTAIN_FAV);
		bytes_used += pDFH_AddMenuItem(pMenuBuffer + bytes_used, s, 
			(WORD) 1, FALSE, TRUE);

		pMenuBuffer[bytes_used++] = 0;
		pMenuBuffer[bytes_used++] = 0;
		// directly after the menu resource, add the "expansion" strings for 
		// menu items, and fix resource IDs or the menu items to be offsets
		// into this "string table"
		memcpy(pMenuBuffer + bytes_used, pStore, nStored);
		if (pShData)
			pShData->dwFavDescOffset = bytes_used;
		ReleaseMutex(m_hResMutex);
	}
	if (pShDataCopy && pShData->dwSize == pShDataCopy->dwSize)
		memcpy(pShDataCopy, pShData, SHMEMSIZE_COPY);

	return bytes_used;
}


BOOL CDragTreeCtrl::RenameItem(HTREEITEM hItem, const TCHAR *szNewName)
{
	BOOL bRet = TRUE;
	TCHAR szPath[MAX_PATH + 20];
	TCHAR ext[20];
	int nImage, m;
	CString sOldFileName = GetFileName(hItem, szPath);

	EnableFolderWatch(FALSE);
	strcpy(ext, _T(".lnk"));
	GetItemImage(hItem, nImage, m);
	if (nImage >= FOLDER_IMAGE) {
		_splitpath(sOldFileName, NULL, NULL, NULL, ext);
	} 

	CString sNewName = EncodeFileName(szNewName);
	if (MoveFile(sOldFileName, szPath + sNewName + ext))
		SetItemText(hItem, szNewName);
	else
		bRet = FALSE;
	EnableFolderWatch(TRUE);
	return bRet;
}


