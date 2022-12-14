#include "stdafx.h"
#include "DFH_shared.h"
#include "DFH.h"

// Use HWND WindowFromDC(HDC hDC); to get hwnd from hDC
typedef BOOL (APIENTRY *DRAWCAPTION) (HWND hwnd, HDC hdc, LPRECT lprc, UINT uFlags);

#pragma pack ( 1 )
typedef struct
{
	BYTE    instr_JMP;
	DWORD   operand_JMP_offset;
} JMP_CODE;

static JMP_CODE InstBuf, OrgDrawCaption;
static DRAWCAPTION pDrawCaption = NULL;
static HANDLE hProcess = NULL;


BOOL CALLBACK InterceptedDrawCaption(HWND hwnd, HDC hdc, LPRECT lprc, UINT uFlags)
{
	WriteProcessMemory(
	  hProcess,  // handle to process whose memory is written to
	  (LPVOID) pDrawCaption,                    // address to start writing to
	  &OrgDrawCaption,  // pointer to buffer to write data to
	  sizeof(JMP_CODE), // number of bytes to write  LPDWORD lpNumberOfBytesWritten 
				// actual number of bytes written);
	  NULL);

	BOOL bRet = pDrawCaption(hwnd, hdc, lprc, uFlags);

	InstBuf.instr_JMP = 0xE9;
	InstBuf.operand_JMP_offset = (DWORD) InterceptedDrawCaption - ((DWORD) pDrawCaption + 5);

	WriteProcessMemory(
	  hProcess,  // handle to process whose memory is written to
	  (LPVOID) pDrawCaption,                    // address to start writing to
	  &InstBuf,  // pointer to buffer to write data to
	  sizeof(JMP_CODE), // number of bytes to write  LPDWORD lpNumberOfBytesWritten 
				// actual number of bytes written);
	  NULL);

	if (pShData) {
		Msg("InterceptedDrawCaption");

		int nphd = (int) GetProp(hwnd, (LPCTSTR) aDfPropName);
		if (nphd > 0) {
			HW_DATA *phd = &pShData->hw[nphd - 1];
			if (phd->bHandleThisWindow)
				Msg(" -- would draw buttons");
				// DrawButtons(phd);
		}
	}    

	return bRet;
}



void InterceptAPIs()
{
	HMODULE hMod = GetModuleHandle("user32.dll");
	if (!hMod) 
		return;

	DWORD dwProcessId = GetCurrentProcessId();
	if (!hProcess) 
		hProcess = OpenProcess(  
			PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION ,  // access flag
			FALSE,    // handle inheritance flag
			dwProcessId       // process identifier);
		);

	if (!hProcess)
		return;

	pDrawCaption = (DRAWCAPTION) GetProcAddress(hMod, "DrawCaption");

	if (!pDrawCaption) {
		Msg("pDrawCaption is NULL!");
		return;
	}

	ReadProcessMemory(
	  hProcess,  // handle to process whose memory is written to
	  (LPVOID) pDrawCaption,                    // address to start writing to
	  &OrgDrawCaption,  // pointer to buffer to write data to
	  sizeof(JMP_CODE), // number of bytes to write  LPDWORD lpNumberOfBytesWritten 
				// actual number of bytes written);
	  NULL);

	InstBuf.instr_JMP = 0xE9;
	InstBuf.operand_JMP_offset = (DWORD) InterceptedDrawCaption - ((DWORD) pDrawCaption + 5);

	WriteProcessMemory(
	  hProcess,  // handle to process whose memory is written to
	  (LPVOID) pDrawCaption,                    // address to start writing to
	  &InstBuf,  // pointer to buffer to write data to
	  sizeof(JMP_CODE), // number of bytes to write  LPDWORD lpNumberOfBytesWritten 
				// actual number of bytes written);
	  NULL);
	Msg("DrawCaption intercepted");
}


void RemoveInterceptAPIs()
{
	if (pDrawCaption)
		WriteProcessMemory(
		  hProcess,  // handle to process whose memory is written to
		  (LPVOID) pDrawCaption,                    // address to start writing to
		  &OrgDrawCaption,  // pointer to buffer to write data to
		  sizeof(JMP_CODE), // number of bytes to write  LPDWORD lpNumberOfBytesWritten 
					// actual number of bytes written);
		  NULL);

	pDrawCaption = NULL;
}

