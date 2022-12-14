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

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B20AAE15_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
#define AFX_STDAFX_H__B20AAE15_1F42_11D3_8563_0000C0597CC7__INCLUDED_

//#define WINVER 0x0500 // ME and 2000
//#define _WIN32_WINNT 0x0410
#define WINVER 0x0501 
#define _WIN32_WINNT 0x0501

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4244 4800 4996) 

// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define OEMRESOURCE
#include <windows.h>
#include <WINDOWSX.H>
#include <tchar.h>
#include <CommDlg.h>
#include <io.h>
#include <stdlib.h>

#include "strsafe.h"
#include <SHELLAPI.H>
#include <shlobj.h>
#include <Shlobj.h>
#include <uxtheme.h>
#include <Tmschema.h>
#include <dwmapi.h>

#ifdef _UNICODE
# define atoi _wtoi
# define sprintf swprintf
# define strlen wcslen
# define strcmp wcscmp
# define strcpy wcscpy
# define strcat wcscat
# define strchr wcschr
# define strncpy wcsncpy
# define strlwr wcslwr
# define strstr wcsstr
# define strncmp wcsncmp
# define _access _waccess
# define _stricmp _wcsicmp
# define stricmp wcsicmp
# define vsprintf vswprintf
#endif


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B20AAE15_1F42_11D3_8563_0000C0597CC7__INCLUDED_)
