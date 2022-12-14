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

// Fbx32helper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Fbx32helper.h"
#include "../DFH/DFH_shared.h"

#define MAX_LOADSTRING 100

typedef void (CALLBACK *MYCALLBACK) ();
typedef DF_SHARED_DATA * (CALLBACK *MYCALLBACK1) (HINSTANCE, DWORD);
typedef DF_SHARED_DATA * (CALLBACK *GetShData_t) ();

DF_SHARED_DATA *pShData = NULL;
DF_SHARED_DATA_COPY *pShDataCopy = NULL;
HINSTANCE hInstDLL = NULL;
GetShData_t GetShData = NULL;
MYCALLBACK pReleaseMsgHooks = NULL;
MYCALLBACK1 pInstallMsgHooks = NULL;
HINSTANCE hInst;								// current instance
bool bHooksOn = false;


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInstDLL = LoadLibrary(_T("FileBXH32.dll"));
	if (hInstDLL == NULL) 
		return 1;
	pInstallMsgHooks = (MYCALLBACK1) GetProcAddress(hInstDLL, "InstallMsgHooks"); 
	pReleaseMsgHooks = (MYCALLBACK) GetProcAddress(hInstDLL, "ReleaseMsgHooks");
	GetShData = (GetShData_t) GetProcAddress(hInstDLL, "GetShData"); 
	if (pInstallMsgHooks == NULL || pReleaseMsgHooks == NULL || GetShData == NULL)
	{
		FreeLibrary(hInstDLL);
		return 1;
	}

	/* Create a named file mapping object */ 
	HANDLE hMapObject = NULL;   /* handle to file mapping */ 

    hMapObject = CreateFileMapping( 
        (HANDLE) INVALID_HANDLE_VALUE, /* use paging file    */ 
        NULL,                /* no security attr.  */ 
        PAGE_READWRITE,      /* read/write access  */ 
        0,                   /* size: high 32-bits */ 
        SHMEMSIZE_COPY,				 /* size: low 32-bits  */ 
        _T("DFolderHook64-32"));      /* name of map object */
    /* The first process to attach initializes memory. */ 
    BOOL fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
    if (hMapObject == NULL || fInit)
	{
		if (hMapObject)
			CloseHandle(hMapObject);
		FreeLibrary(hInstDLL);
        return 0; 
	}

    /* Get a pointer to the file-mapped shared memory. */ 
    pShDataCopy = (DF_SHARED_DATA *) MapViewOfFile( 
        hMapObject,     /* object to map view of    */ 
        FILE_MAP_WRITE, /* read/write access        */ 
        0,              /* high offset:   map from  */ 
        0,              /* low offset:    beginning */ 
        0);             /* default: map entire file */ 
    if (pShDataCopy == NULL)
	{
		CloseHandle(hMapObject);
		FreeLibrary(hInstDLL);
        return FALSE; 
	}

	//Sleep(2000);
	pShData = pInstallMsgHooks(hInstDLL, 0);
	pShData->dwSize = SHMEMSIZE;
	
	if (pShData && pShData->dwSize == pShDataCopy->dwSize)
	{
		bHooksOn = true;
		memcpy(pShData, pShDataCopy, SHMEMSIZE_COPY);
		UINT_PTR timer = SetTimer(NULL, 0, 1000, NULL);

		// Main message loop:
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) && pShData && pShData->hDFhWnd && ::IsWindow(pShData->hDFhWnd))
		{
			memcpy(pShData, pShDataCopy, SHMEMSIZE_COPY);
			if (!bHooksOn && (pShData->dwButtons  & 0x7fff))
			{
				pInstallMsgHooks(hInstDLL, 0);
				bHooksOn = true;
			}
			else if (bHooksOn && (pShData->dwButtons  & 0x7fff) == 0)
			{
				pReleaseMsgHooks();
				bHooksOn = false;
			}
		}
	}
	if (pShData)
		pReleaseMsgHooks();
	UnmapViewOfFile(pShDataCopy);
	CloseHandle(hMapObject);
	FreeLibrary(hInstDLL);
	return 0;
}

