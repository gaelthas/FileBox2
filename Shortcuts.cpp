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

// Shortcuts.cpp
//

#include "stdafx.h"
#include "FileBX.h"
#include "DFH/DFH_shared.h"

extern "C" {
#include <shlobj.h>
#include <objbase.h>
}

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL b_CoInitialize = FALSE;


HGLOBAL CreateHDrop(int cnt, TCHAR *names[])
{
	HGLOBAL hGlobal;
	// for my sample I am assuming number of files as 2 
 	// int cnt = 2;
	// allocate space for DROPFILE structure plus the number of file and one extra byte for final NULL terminator
	hGlobal = GlobalAlloc(GHND|GMEM_SHARE,(DWORD) (sizeof(DROPFILES)+(_MAX_PATH)*cnt+1));
	if(hGlobal == NULL)
			return hGlobal;

	LPDROPFILES pDropFiles;
	pDropFiles = (LPDROPFILES)GlobalLock(hGlobal);
	// set the offset where the starting point of the file start
   	pDropFiles->pFiles = sizeof(DROPFILES);
	// file contains wide characters
   	pDropFiles->fWide=FALSE;

	int CurPosition = sizeof(DROPFILES);
	for (int i = 0;i < cnt;i++)
	{
		TCHAR szFileName[_MAX_PATH];
		//Get File Name
		strcpy(szFileName, names[i]);
                // copy the file into global memory
		lstrcpy((LPTSTR)((LPTSTR)(pDropFiles)+CurPosition), szFileName);
		// move the current position beyond the file name copied
		// don't forget the Null terminator (+1)
		CurPosition += (unsigned int)strlen(szFileName)+1;
	 }
	// final null terminator as per CF_HDROP Format specs.
	((LPSTR)pDropFiles)[CurPosition]=0;
	GlobalUnlock(hGlobal);
 	return hGlobal;
}


HRESULT CreateShortCut(LPCTSTR pszTargetFile, LPCTSTR pszLink, 
  LPTSTR pszDesc /* = NULL */)
// pszTargetFile is original file path
// pszLink is FileName.lnk in target folder
// pszDesc seems to do nothing...
{
    HRESULT hres;
    IShellLink* psl;

	if (! b_CoInitialize) {
		CoInitialize(NULL);
		b_CoInitialize = TRUE;
	}

	// Get a pointer to the IShellLink interface.
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **) &psl);
    if (SUCCEEDED(hres))
    {
       IPersistFile* ppf;

       // Query IShellLink for the IPersistFile interface for 
       // saving the shell link in persistent storage.
       hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
       if (SUCCEEDED(hres))
       {   
         WORD wsz[MAX_PATH];

         // Set the path to the shell link target.
         hres = psl->SetPath(pszTargetFile);

         if (!SUCCEEDED(hres))
           AfxMessageBox(_T("SetPath failed!"));

         // Set the description of the shell link.
		 if (pszDesc) {
	         hres = psl->SetDescription(pszDesc);

			if (!SUCCEEDED(hres))
				AfxMessageBox(_T("SetDescription failed!"));
		 }

#ifndef _UNICODE
         // Ensure string is ANSI.
         MultiByteToWideChar(CP_ACP, 0, pszLink, -1, (LPWSTR) wsz, MAX_PATH);
#else
		 strcpy((TCHAR*)wsz, pszLink);
#endif

         // Save the link via the IPersistFile::Save method.
         hres = ppf->Save((LPCOLESTR) wsz, TRUE);
    
         // Release pointer to IPersistFile.
         ppf->Release();
       }
       // Release pointer to IShellLink.
       psl->Release();
    }
    return hres;
}


HRESULT ResolveShortCut(HWND hwnd, LPCTSTR pszShortcutFile, 
						WIN32_FIND_DATA &wfd, LPTSTR pszPath /* = NULL */,
						LPTSTR pszDescription /* = NULL */, BOOL bDoResolve /* = FALSE*/, 
						LPTSTR pszWorkDir /* = NULL */,
						LPTSTR pszArgs /* = NULL */,
						int   *piShowCmd /* = NULL */)
// pszPath must be at least MAX_PATH long, receives the path name (without file name)
{
  HRESULT hres;
  IShellLink* psl;
  TCHAR szGotPath[MAX_PATH];
  // char szDescription[MAX_PATH];
  // WIN32_FIND_DATA wfd;

	if (! b_CoInitialize) {
		CoInitialize(NULL);
		b_CoInitialize = TRUE;
	}

  if (pszPath)
	  *pszPath = 0;   // assume failure

  // Get a pointer to the IShellLink interface.
  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **) &psl);
  if (SUCCEEDED(hres))
  {
    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface.
    hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
    if (SUCCEEDED(hres))
    {
      WORD wsz[MAX_PATH];

#ifdef _UNICODE
	  strcpy((TCHAR*) wsz, pszShortcutFile);
#else
      // Ensure string is Unicode.
      MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, (LPWSTR) wsz,
                                 MAX_PATH);
#endif

     // Load the shell link.
     hres = ppf->Load((LPCOLESTR) wsz, STGM_READ);
     if (hres == NOERROR)
     {
       // Resolve the link.
	   hres = bDoResolve ? psl->Resolve(hwnd, SLR_NO_UI | SLR_UPDATE) : 0;

       if (SUCCEEDED(hres))
       {
          strcpy(szGotPath, pszShortcutFile);
          // Get the path to the link target.
          hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, 
                 0 );
          if (!SUCCEEDED(hres))
             AfxMessageBox(_T("GetPath failed!"));
		  if (pszPath)
	          strcpy(pszPath, szGotPath);

          // Get the description of the target.         
		  if (pszDescription) {
			  *pszDescription = 0;
			  hres = psl->GetDescription(pszDescription, MAX_PATH);
			  if (!SUCCEEDED(hres))
				 AfxMessageBox(_T("GetDescription failed!"));
		  }
		  if (pszWorkDir) {
			  *pszWorkDir = 0;
			  hres = psl->GetWorkingDirectory(pszWorkDir, MAX_PATH);
		  }
		  if (pszArgs) {
			  *pszArgs = 0;
			  hres = psl->GetArguments(pszArgs, MAX_PATH);
		  }
		  if (piShowCmd) {
			  *piShowCmd = SW_SHOWNORMAL;
			  hres = psl->GetShowCmd(piShowCmd);
		  }
        }
      }
      // Release pointer to IPersistFile interface.
      ppf->Release();
    }
    // Release pointer to IShellLink interface.
    psl->Release();
  }
  return hres;
}


HRESULT SetItemNumber(HWND hwnd, LPCTSTR pszShortcutFile, int nItemNo)
{
  HRESULT hres;
  IShellLink* psl;

	if (! b_CoInitialize) {
		CoInitialize(NULL);
		b_CoInitialize = TRUE;
	}

  // Get a pointer to the IShellLink interface.
  hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void **) &psl);
  if (SUCCEEDED(hres))
  {
    IPersistFile* ppf;

    // Get a pointer to the IPersistFile interface.
    hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
    if (SUCCEEDED(hres))
    {
      WORD wsz[MAX_PATH];

#ifdef _UNICODE
	  strcpy((LPWSTR) wsz, pszShortcutFile);
#else
      // Ensure string is Unicode.
      MultiByteToWideChar(CP_ACP, 0, pszShortcutFile, -1, (LPWSTR) wsz,
                                 MAX_PATH);
#endif

     // Load the shell link.
     hres = ppf->Load((LPCOLESTR) wsz, STGM_READ);
     if (SUCCEEDED(hres))
     {
		 TCHAR buf[20];
		 wsprintf(buf, _T("HyperionicsDF %d"), nItemNo);
		 hres = psl->SetDescription(buf);
		 if (SUCCEEDED(hres))
			 ppf->Save((LPCOLESTR) wsz, TRUE);
		 else
			 AfxMessageBox(_T("SetDescription failed!"));

      }
      // Release pointer to IPersistFile interface.
      ppf->Release();
    }
    // Release pointer to IShellLink interface.
    psl->Release();
  }
  return hres;
}


BOOL IsFolder(const TCHAR *pszFile)
{
	WIN32_FIND_DATA wfd;
	HANDLE hff;
	BOOL bRet = FALSE;

	if (strpbrk(pszFile, _T("*?")))
		return FALSE;

	hff = FindFirstFile(pszFile, &wfd);
	if (hff != INVALID_HANDLE_VALUE) {
		bRet = (BOOL) (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		FindClose(hff);
	} else {
		TCHAR buf[MAX_PATH + 20];
		GetCurrentDirectory(MAX_PATH, buf);
		if (SetCurrentDirectory(pszFile)) {
			SetCurrentDirectory(buf);
			return TRUE;
		}
	}

	return bRet;
}


BOOL MoveFileOrFolder(const TCHAR *szSrc, const TCHAR *szTarget)
// Move entire directory tree by copy/delete, needed e.g.
// when they reside on different drives.
{
	BOOL bRet = TRUE;

    SHFILEOPSTRUCT FileOp;
	memset(&FileOp, 0, sizeof(FileOp));
	TCHAR buf[MAX_PATH + 20];
	memset(buf, 0, sizeof(buf));
	strcpy(buf, szSrc);

	FileOp.hwnd = NULL; 
	FileOp.wFunc = szTarget ? FO_MOVE : FO_DELETE;
	FileOp.pFrom = buf; 
	FileOp.pTo = szTarget; 
	FileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_RENAMEONCOLLISION | FOF_SIMPLEPROGRESS; 
	/*
	FileOp.fAnyOperationsAborted; 
	FileOp.hNameMappings; 
	*/
	FileOp.lpszProgressTitle = _T(""); 

	bRet = !SHFileOperation(&FileOp);

	return bRet;
}



int ReadIcoToResource(TCHAR *fname, BYTE *pBytes, int nMaxSize)
// Returns the size in bytes that the resource occupies
{
	FILE *fp;
	fp = fopen(fname, _T("rb"));
	if (!fp)
		return 0;
	int nSize = 0;
	ICONDIR	id, *pid;
	fread(&id, 1, sizeof(id), fp);
	int n = sizeof(ICONDIR) + (id.idCount - 1) * sizeof(ICONDIRENTRY);
	int ns;

	pid = (ICONDIR *) malloc(n);
	if (pid) {
		rewind(fp);
		fread(pid, 1, n, fp);
		GRPICONDIR *pgid = (GRPICONDIR *) pBytes;
		pgid->idReserved = 0;
		pgid->idType = 1;
		pgid->idCount = pid->idCount;
		for (n = 0; n < pid->idCount; n++) {
			pgid->idEntries[n].bWidth = pid->idEntries[n].bWidth;
			pgid->idEntries[n].bHeight = pid->idEntries[n].bHeight;
			pgid->idEntries[n].bColorCount = ns = pid->idEntries[n].bColorCount;
			pgid->idEntries[n].bReserved = pid->idEntries[n].bReserved;
			pgid->idEntries[n].wPlanes = 1;
			pgid->idEntries[n].wBitCount = (ns <= 2 ? 1 :(ns <= 16 ? 4 : 8));
			pgid->idEntries[n].dwBytesInRes = pid->idEntries[n].dwBytesInRes;
			pgid->idEntries[n].nID = 1000+n;
		}
		nSize = sizeof(GRPICONDIR) + (pgid->idCount - 1)*sizeof(GRPICONDIRENTRY);

		for (n = 0; n < pid->idCount; n++) {
			fseek(fp, pid->idEntries[n].dwImageOffset, SEEK_SET);
			ns = pid->idEntries[n].dwBytesInRes;
			if (nSize + ns > nMaxSize) {
				nSize = 0;
				break;
			}
			if (fread(pBytes + nSize, 1, ns, fp) < (size_t) ns) {
				nSize = 0;
				break;
			}
			nSize += ns;
		}

		free(pid);
	}

	fclose(fp);
	if (!nSize)
		memset(pBytes, 0, nMaxSize);
	return nSize;
}


CString EncodeFileName(const TCHAR *inStr)
{
	TCHAR outStr[MAX_PATH + 30], *pc, s[4];
	int ch;
	strcpy(outStr, inStr);
	pc = outStr;
	while (pc = strpbrk(pc, _T("%*?\"\\/:"))) {
		ch = *pc;
		memmove(pc+2, pc, strlen(pc)+1); // need final 0 as well
		wsprintf(s, _T("%%%X"), ch);
		strncpy(pc, s, 3);
		pc += 3;
	}

	return CString(outStr);
}


CString DecodeFileName(const TCHAR *inStr)
{
	TCHAR outStr[MAX_PATH + 30], *pc, s[4];
	int ch;
	strcpy(outStr, inStr);
	pc = outStr;
	while (pc = strchr(pc, '%')) {
		strncpy(s, pc+1, 2);
		s[2] = 0;
		sscanf(s, _T("%x"), &ch);
		*pc = ch;
		memmove(pc+1, pc+3, strlen(pc+2)); // need final 0 as well
		pc++;
	}

	return CString(outStr);
}


class CDirTime {
public:
	CString m_Dir;
	FILETIME ftLastAccessTime;
};

class CDirTimes : public CArray <CDirTime, CDirTime&> {
	static int Compare(const void *, const void *);
	static int CompareNameTime(const void *, const void *);
public:
	void SortByTime() {qsort(GetData(), GetSize(), sizeof(CDirTime), Compare);};
	void SortByNameTime() {qsort(GetData(), GetSize(), sizeof(CDirTime), CompareNameTime);};
};

int CDirTimes::Compare(const void *arg1, const void *arg2) 
{
	CDirTime p1=*(CDirTime *)arg1;
	CDirTime p2=*(CDirTime *)arg2;
	if (p1.ftLastAccessTime.dwHighDateTime > p2.ftLastAccessTime.dwHighDateTime)
		return -1;
	else if (p1.ftLastAccessTime.dwHighDateTime < p2.ftLastAccessTime.dwHighDateTime)
		return +1;
	if (p1.ftLastAccessTime.dwLowDateTime > p2.ftLastAccessTime.dwLowDateTime)
		return -1;
	else if (p1.ftLastAccessTime.dwLowDateTime < p2.ftLastAccessTime.dwLowDateTime)
		return +1;
	return 0;
}

int CDirTimes::CompareNameTime(const void *arg1, const void *arg2) 
{
	CDirTime p1=*(CDirTime *)arg1;
	CDirTime p2=*(CDirTime *)arg2;
	CString s1 = p1.m_Dir, s2 = p2.m_Dir;
	s1.MakeLower(); s2.MakeLower();
	if (s1 > s2)
		return -1;
	else if (s1 < s2)
		return +1;
	else if (p1.ftLastAccessTime.dwHighDateTime > p2.ftLastAccessTime.dwHighDateTime)
		return -1;
	else if (p1.ftLastAccessTime.dwHighDateTime < p2.ftLastAccessTime.dwHighDateTime)
		return +1;
	if (p1.ftLastAccessTime.dwLowDateTime > p2.ftLastAccessTime.dwLowDateTime)
		return -1;
	else if (p1.ftLastAccessTime.dwLowDateTime < p2.ftLastAccessTime.dwLowDateTime)
		return +1;
	return 0;
}

BOOL FillRecentFoldersBuffer()
{
	if (!pShData || !pShData->bRecentFromWin)
		return FALSE;

	// Get recent folders from reg key:
	// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders
	// Value name: Recent

	HKEY hk;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, 
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
		0, KEY_READ,
		&hk);
	if (lRes != ERROR_SUCCESS)
		return FALSE;
	DWORD dwType;
	TCHAR szRecentFolder[MAX_PATH+20];
	DWORD dwSize = sizeof(szRecentFolder)-10;
	lRes = RegQueryValueEx(
		hk,
		_T("Recent"),
		NULL,
		&dwType,
		(LPBYTE) szRecentFolder,
		&dwSize
		);
	RegCloseKey(hk);
	if (lRes != ERROR_SUCCESS || !IsFolder(szRecentFolder))
		return FALSE;

	WIN32_FIND_DATA wfd;
	HANDLE hff;
	BOOL bRet = FALSE;

	CString sRecentFolder(szRecentFolder);
	sRecentFolder += _T("\\");
	hff = FindFirstFile(sRecentFolder + _T("*.lnk"), &wfd);
	if (hff == INVALID_HANDLE_VALUE) 
		return FALSE;

	HRESULT hres;
	IShellLink* psl;
	TCHAR szGotPath[MAX_PATH];
	WIN32_FIND_DATA wfd2;

	if (! b_CoInitialize) {
		CoInitialize(NULL);
		b_CoInitialize = TRUE;
	}

	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_IShellLink, (void **) &psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface.
		hres = psl->QueryInterface(IID_IPersistFile, (void **) &ppf);
		if (SUCCEEDED(hres))
		{
			WORD wsz[MAX_PATH];
			CDirTime dt;
			CDirTimes dts;
			dts.SetSize(256);
			dts.RemoveAll();

			do {
#ifdef _UNICODE
				strcpy((LPWSTR) wsz, sRecentFolder + wfd.cFileName);
#else
				// Ensure string is Unicode.
				MultiByteToWideChar(CP_ACP, 0, sRecentFolder + wfd.cFileName, -1, (LPWSTR) wsz,
					MAX_PATH);
#endif

				// Load the shell link.
				hres = ppf->Load((LPCOLESTR) wsz, STGM_READ);
				if (hres == NOERROR)
				{
					strcpy(szGotPath, sRecentFolder + wfd.cFileName);
					// Get the path to the link target.
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd2, 0);
					if (hres == NOERROR && wfd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (_access(szGotPath, 0) < 0)
							DeleteFile(sRecentFolder + wfd.cFileName);
						else
						{
							dt.m_Dir = szGotPath;
							dt.ftLastAccessTime = wfd.ftLastAccessTime;
							dts.Add(dt);
						}
					}
				}
			} while (FindNextFile(hff, &wfd));

			// Lets add to the mix our own "recent" folders from pShData->sRecent buffer...
			int i;
			for (i = 0; i < pShData->nRecent; i++)
			{
				if (_access(pShData->sRecent[i], 0) >= 0)
				{
					dt.m_Dir = pShData->sRecent[i];
					HANDLE hFile;	 
					hFile = CreateFile(szGotPath,    // file to open
									   GENERIC_READ,          // open for reading
									   FILE_SHARE_READ,       // share for reading
									   NULL,                  // default security
									   OPEN_EXISTING,         // existing file only
									   FILE_FLAG_BACKUP_SEMANTICS, // needed for dir reading
									   NULL);                 // no attr. template
					 
					if (hFile != INVALID_HANDLE_VALUE)
					{ 
						GetFileTime(hFile, NULL, &dt.ftLastAccessTime, NULL);
						CloseHandle(hFile);
						if (dt.m_Dir.Right(1) == "\\")
							dt.m_Dir = dt.m_Dir.Left(dt.m_Dir.GetLength() - 1);
					}
					dts.Add(dt);
				}
			}

			// - Eliminate duplicates
			dts.SortByNameTime();
			for (i = 1; i < dts.GetCount(); i++) {
				CString s1 = dts[i-1].m_Dir, s2 = dts[i].m_Dir;
				s1.MakeLower(); s2.MakeLower();
				if (s1 == s2)
					dts.RemoveAt(i--);
			}
			dts.SortByTime();
			//Msg("----------------------------------------------");
			for (i = 0; i < dts.GetCount() && i < pShData->nRecent; i++) {
				dt = dts[i];
				strncpy(pShData->sRecent[i], dt.m_Dir, MAX_PATH-1);
				pShData->sRecent[i][MAX_PATH-1] = 0;
				Msg(_T("%u %u %s"), dt.ftLastAccessTime.dwHighDateTime, dt.ftLastAccessTime.dwLowDateTime, dt.m_Dir);
			}
			pShData->nRecent = i;
			//Msg("----------------------------------------------");
			if (pShDataCopy && pShData->dwSize == pShDataCopy->dwSize)
				memcpy(pShDataCopy, pShData, SHMEMSIZE_COPY);			
		}
	}
	FindClose(hff);
	return TRUE;
}
