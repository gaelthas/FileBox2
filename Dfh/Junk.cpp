// Junk

static int nRecursionCount = 0;

LRESULT CALLBACK HookedWindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	HW_DATA *phd = (HW_DATA *) GetProp(hwnd, (LPCTSTR) aDfPropName);
	LRESULT lRet;
	WNDPROC OldHookedWindowProc = phd->OldHookedWindowProc;
	RECT rctBtn, rctWin;
	HDC hdc;
	int xPos, yPos;

	if (phd->dwRelTime1 && GetTickCount() - phd->dwRelTime1 > 100) {
		phd->bPushed1 = FALSE;
		phd->dwRelTime1 = 0;
	}

	nRecursionCount++;
	memcpy(&rctBtn, &phd->rctBtn, sizeof(RECT)); // in case we delete phd...
	memcpy(&rctWin, &phd->rctWin, sizeof(RECT)); // in case we delete phd...

	switch (uMsg) {
	case WM_DESTROY:
		RemoveLocalHooks(phd);
		break;

	case WM_MOVE:
	case WM_SIZE:
	case WM_NCACTIVATE:
	case WM_NCPAINT:
		{
			GetWindowRect(hwnd, &phd->rctWin);
			if (!phd->iButtonsLeft) {
				POINT p;
				p.x = phd->rctWin.right;
				p.y = phd->rctWin.top + 16;
				int nhits = 0, xhit;
				do {
					p.x -= 4;
					lRet = CallWindowProc(OldHookedWindowProc, hwnd, WM_NCHITTEST, 0, (LPARAM) (p.y << 16 | p.x));
					if (lRet == HTCAPTION) {
						nhits++;
						if (nhits == 1)
							xhit = p.x;
					} else
						nhits = 0;
				} while (nhits < 3 && p.x >= 0);
				if (nhits == 3) {
					p.x = xhit;
					do {
						p.x++;
						lRet = CallWindowProc(OldHookedWindowProc, hwnd, WM_NCHITTEST, 0, (LPARAM) (p.y << 16 | p.x));
					} while (lRet == HTCAPTION);
					phd->iButtonsLeft = p.x - phd->rctWin.left;
				}
			}

			LONG lExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
			int iBtnWidth = GetSystemMetrics(SM_CXSIZE) - 2;
			int iBtnHeight = GetSystemMetrics(SM_CYSIZE) - 4;
			if (phd->iButtonsLeft)
				phd->rctBtn.left = phd->iButtonsLeft - iBtnWidth;
			else {
				phd->rctBtn.left = phd->rctWin.right - phd->rctWin.left - 6 - 3*iBtnWidth - GetSystemMetrics(SM_CXDLGFRAME);
				if (lExStyle & WS_EX_CONTEXTHELP)
					phd->rctBtn.left -= iBtnWidth + 4;
				if (lStyle & WS_MAXIMIZEBOX)
					phd->rctBtn.left -= iBtnWidth;
				if (lStyle & WS_MINIMIZEBOX)
					phd->rctBtn.left -= iBtnWidth;
			}

			phd->rctBtn.top = GetSystemMetrics((lStyle & WS_THICKFRAME) ? SM_CYSIZEFRAME : SM_CYDLGFRAME)+2;
			phd->rctBtn.bottom = phd->rctBtn.top + iBtnHeight;
			phd->rctBtn.right = phd->rctBtn.left + iBtnWidth;
			memcpy(&rctBtn, &phd->rctBtn, sizeof(RECT)); // in case we delete phd...
		}
		break;

	case WM_NCLBUTTONDOWN:
		xPos = GET_X_LPARAM(lParam) - rctWin.left;  // horizontal position of cursor 
		yPos = GET_Y_LPARAM(lParam) - rctWin.top;  // vertical position of cursor
		POINT pt; pt.x = xPos; pt.y = yPos;
		if (!phd->bPushed1 && PtInRect(&rctBtn, pt)) {
			int nEditId = phd->nEditId; // in case we delete phd...
			hdc = GetWindowDC(hwnd);
			DrawFrameControl(hdc, &rctBtn, DFC_BUTTON, 
				DFCS_BUTTONPUSH|DFCS_PUSHED);
			phd->bPushed1 = TRUE;
			HMENU hMenu = CreatePopupMenu();
			char *pcDirs[] = {"C:\\", "C:\\tmp\\", "E:\\tmp\\", "Quit", NULL};

			for (int i = 0; pcDirs[i]; i++)
				AppendMenu(hMenu, MF_STRING, 100+i, pcDirs[i]);
			int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, 
				rctBtn.left + rctWin.left, rctBtn.bottom + rctWin.top, 
				0, hwnd, NULL);
			DestroyMenu(hMenu);

			switch (iRet) {
			case 100:
			case 101:
			case 102:
				{
					char buf[512];
					HWND hCtl = NULL;
					if (!(hCtl = GetDlgItem(hwnd, nEditId))) {
						Msg("Not Found!");
						break;
					}
					GetWindowText(hCtl, buf, 512);
					SetWindowText(hCtl, pcDirs[iRet-100]);
					if (GetDlgItem(hwnd, IDOK)) {
						SendDlgItemMessage(hwnd, IDOK, BM_CLICK, 0, 0);
					} else {
						SendMessage(hCtl, WM_KEYDOWN, VK_RETURN, 0);
						SendMessage(hCtl, WM_KEYUP, VK_RETURN, 0);
					}
					SetWindowText(hCtl, buf);
				}
				break;
			case 103: // Quit
				RemoveLocalHooks(NULL);
				break;
			}

			DrawFrameControl(hdc, &rctBtn, DFC_BUTTON, DFCS_BUTTONPUSH);
			ReleaseDC(hwnd, hdc);
			if (GetProp(hwnd, (LPCTSTR) aDfPropName))
				phd->dwRelTime1 = GetTickCount();

			// -- The below needed for regular command button
			// phd->bPushed1 = TRUE;
			// SetCapture(hwnd);
			lRet = 0;
			goto Finish;
		}
		break;

	case WM_LBUTTONUP:
		if (phd->bPushed1) {
			hdc = GetWindowDC(hwnd);
			DrawFrameControl(hdc, &rctBtn, DFC_BUTTON, DFCS_BUTTONPUSH);
			ReleaseDC(hwnd, hdc);
			ReleaseCapture();
			phd->bPushed1 = FALSE;

			xPos = GET_X_LPARAM(lParam);  // horizontal position of cursor 
			yPos = GET_Y_LPARAM(lParam);  // vertical position of cursor
			POINT pt; pt.x = xPos; pt.y = yPos;
			ClientToScreen(hwnd, &pt);
			pt.x -= rctWin.left;
			pt.y -= rctWin.top;
			if (PtInRect(&rctBtn, pt)) {
				// execute btn action
			}
			lRet = 0;
			goto Finish;
		}
		break;
	}

	lRet = CallWindowProc(OldHookedWindowProc, hwnd, uMsg, wParam, lParam);

	if (pHwFirst && (uMsg == WM_NCACTIVATE || uMsg == WM_NCPAINT)) {
		hdc = GetWindowDC(hwnd);
		DrawFrameControl(hdc, &rctBtn, DFC_BUTTON, DFCS_BUTTONPUSH);
		ReleaseDC(hwnd, hdc);
	}

Finish:
	nRecursionCount--;
	return lRet;
}


LRESULT CALLBACK DF_CBTHookProc(
    int nCode,	// hook code
    WPARAM wParam,	// current-process flag
    LPARAM lParam 	// address of structure with message data
   )
{
	if (nCode == HCBT_ACTIVATE && !hHookWndProcRet && *hhkcb) {
		char buf[512];
		GetModuleFileName(hDllInst, buf, 512);
		hDllInst = LoadLibrary(buf); // to increase use count
		HINSTANCE hUnloader = GetModuleHandle("e:\\src\\dfolder\\debug\\unloader.dll");
		if (!hUnloader)
			hUnloader = LoadLibrary("e:\\src\\dfolder\\debug\\unloader.dll");
		pUnloader = (UNLOADER) GetProcAddress(hUnloader, "Unloader");
		Msg("pUnloader is 0x%X", pUnloader);

		nRecursionCount = 0;
		hHookWndProcRet = hHookWndProcRet2 = SetWindowsHookEx(
		  WH_CALLWNDPROCRET,    // type of hook to install
		  CallWndRetProc,		// pointer to hook procedure
		  hDllInst,				// handle to application instance
		  GetCurrentThreadId()	// identity of thread to install hook for
		);
		if (*hhkcb && ! hhkcb_local)
			hhkcb_local = *hhkcb;
	}

	return CallNextHookEx(
		hhkcb_local,	// handle to current hook
		nCode,	// hook code passed to hook procedure
		wParam,	// value passed to hook procedure
		lParam 	// value passed to hook procedure
	   );
}



LRESULT CALLBACK MouseProc2 (int nCode, WPARAM wParam, LPARAM lParam )
{
	LPMOUSEHOOKSTRUCT pM = (MOUSEHOOKSTRUCT *) lParam;
	if (nCode >= 0)  {
		switch (wParam) {
		case WM_NCLBUTTONDOWN:		
		case WM_LBUTTONDOWN:
			char buf[256], bufP[256];
			HWND hwnd, h, hwndParent = NULL;
			*buf = *bufP = 0;
			if (pM->hwnd) {
				POINT pt;
				pt.x = pM->pt.x;
				pt.y = pM->pt.y;
				Msg("WindowFromPoint(pt): 0x%X, Mouse hwnd: 0x%X", WindowFromPoint(pt), pM->hwnd);

				hwnd = pM->hwnd; h = NULL;
				ScreenToClient(hwnd, &pt);
				while ((h = ChildWindowFromPoint(hwnd, pt)) && h != hwnd) {
					hwnd = h;
					pt.x = pM->pt.x;
					pt.y = pM->pt.y;
					ScreenToClient(hwnd, &pt);
				}
				GetWindowText(hwnd, buf, 256);
				if (hwndParent = GetParent(hwnd))
					GetWindowText(hwndParent, bufP, 256);
			}
			int nID = GetDlgCtrlID(hwnd);
			int nIDParent = GetDlgCtrlID(hwndParent);
			Msg("Mouse on hwnd = 0x%X, nID = 0x%X, [%s], parent = 0x%X, [%s], nID = 0x%X", hwnd, nID, buf, hwndParent, bufP, nIDParent);
			
			COPYDATASTRUCT cds;
			cds.cbData = (DWORD) hwndParent;
			cds.dwData = 0;
			cds.lpData = NULL;
			// wParam with the handle of our own window will mark the debug msg
			SendMessage(pShData->hDFhWnd, WM_COPYDATA, (WPARAM) hwnd, (LPARAM) &cds);

			UnhookWindowsHookEx( *pMouseHook );	// undoes Set
			*pMouseHook = NULL;
			return TRUE; // prevent from processing
			break;
			
		}
   }
   return( CallNextHookEx(*pMouseHook, nCode, wParam, lParam));
}


DF_SHARED_DATA * CALLBACK SetMouseHook(HINSTANCE hInstDLL)
{
	if (!(*pMouseHook)) {
		*pMouseHook = SetWindowsHookEx( WH_MOUSE,        // install mouse message hook
									&MouseProc,     // address of mouse hook procedure
									hInstDLL,			// handle to dll instance
									(DWORD) 0 );	// hook procedure is associated with all existing threads. 
	}
	return pShData;
}


void CALLBACK ReleaseMouseHook(void)
{
	if (*pMouseHook) {
		UnhookWindowsHookEx( *pMouseHook );	// undoes Set
		*pMouseHook = NULL;
	}
}

