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

#if !defined(AFX_DRAGTREECTRL_H__61F64EE6_4692_11D3_859C_0000C0597CC7__INCLUDED_)
#define AFX_DRAGTREECTRL_H__61F64EE6_4692_11D3_859C_0000C0597CC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DragTreeCtrl.h : header file
//

class  CDragTreeCtrl;

class COleDropSourceDF : public COleDropSource
{
public:
	COleDropSourceDF();       

	CDragTreeCtrl* m_pTree;

// Overrides
	SCODE GiveFeedback(DROPEFFECT /*dropEffect*/);
};


class CTreeDropTarget : public COleDropTarget
{
// Construction
public:
	CTreeDropTarget();           // protected constructor used by dynamic creation

// Implementation
public:
	virtual ~CTreeDropTarget();

// Attributes
	UINT m_ImageFormat;
	DROPEFFECT m_dropEffectCurrent;

   //
   // These members MUST be overridden for an OLE drop target
   // See DRAG and DROP section of OLE classes reference
   //
   DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD 
                                                dwKeyState, CPoint point );
   DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD 
                                               dwKeyState, CPoint point );
   void OnDragLeave(CWnd* pWnd);               
   
   BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT 
                                          dropEffect, CPoint point );  

//protected:
//    IDropTargetHelper* m_piDropHelper;
//    bool               m_bUseDnDHelper;

};


/////////////////////////////////////////////////////////////////////////////
// CDragTreeCtrl window

class CDragTreeCtrl : public CTreeCtrl
{
	friend class COleDropSourceDF;
	friend class CTreeDropTarget;

// Construction
public:
	CDragTreeCtrl();

// Attributes
public:
	COleDropSourceDF m_OleDropSource;
	CTreeDropTarget  m_OleDropTarget;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDragTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL		RenameItem(HTREEITEM hItem, const TCHAR *szNewName);
	DWORD		CreateMenuResource(BYTE *pMenuBuffer, DWORD dwBufLen, 
					HTREEITEM hParent = TVI_ROOT);
	virtual		~CDragTreeCtrl();
	void		RenumberItems(HTREEITEM hParentItem);
	CString		GetFileName(HTREEITEM hItem, TCHAR *pszPath = NULL);
	TCHAR		m_pszFolder[MAX_PATH + 10];
	void		RefreshTree(HTREEITEM hParent = TVI_ROOT, TCHAR *pszFolder = NULL, BOOL bResolveLinks = FALSE);
	void		EnableFolderWatch(BOOL bEnable);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDragTreeCtrl)
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg	LRESULT OnHandleDrag(WPARAM pDataSource, LPARAM lParam = 0);
	afx_msg LRESULT CDragTreeCtrl::OnRefreshMessage(WPARAM, LPARAM);
	
	void CDragTreeCtrl::OnDropFiles(CString sNames[], int nFiles);
	void OnDragOver(COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	void DrawInsertLine(int x = -10001, int y = -10001);
	void MoveTree(HTREEITEM hDest, HTREEITEM hSrc, HTREEITEM hInsertAfter = TVI_LAST);
	void CopyTree(HTREEITEM hDest, HTREEITEM hSrc, HTREEITEM hInsertAfter = TVI_LAST);
	void CopyChildren(HTREEITEM hDest, HTREEITEM hSrc);
	static UINT ThreadFunc(LPVOID pParam);

	CImageList	*m_pDrImList;
	BOOL		m_bImageHidden;
	HTREEITEM	m_htiTarget;	// handle to target item 
	HTREEITEM	m_hDragItem;
	int			m_iInsert;		// -1 nowhere, 0 before, 1 on, 2 after
	int			x_old_line, y_old_line;
	BOOL		m_bDropRegistered;
	HANDLE		m_hChange;
	HANDLE		m_hResMutex;

	DECLARE_MESSAGE_MAP()
public:
};

#define FAV_FOLDER_IMAGE 0
#define DOC_IMAGE 1
#define FOLDER_IMAGE 2
#define TEXT_IMAGE 3
#define	SEP_IMAGE 4

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAGTREECTRL_H__61F64EE6_4692_11D3_859C_0000C0597CC7__INCLUDED_)
