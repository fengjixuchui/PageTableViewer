#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "main.h"
#include "driver_loader.h"
#include "gui_viewer.h"

HANDLE hDriverFile;
static HANDLE hProcessEv = INVALID_HANDLE_VALUE;

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) 
{
	MSG msg;
	HWND hPTV;
	
	char szDriverPath[0x400];
	
	GetDriverPath(NULL, szDriverPath, sizeof(szDriverPath) / sizeof(char));
	hDriverFile = CreateDriverFile(DEVICE_NAME, CheckExecutingSameProcess() ? NULL : DRIVER_NAME, szDriverPath);
	
	hPTV = CreatePTViewerWindow(hInstance);
	
	while (GetMessage(&msg, NULL, 0, 0) > 0){
		if (IsDialogMessage(hPTV, &msg))
			continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	CloseDriverFile(hDriverFile, CheckExecutingSameProcess() ? NULL : DRIVER_NAME);
	
	if (hProcessEv != INVALID_HANDLE_VALUE)
		CloseHandle(hProcessEv);
	
	return 0;
}

void GetDriverPath(HMODULE hModule, LPTSTR lpFilename, DWORD nSize)
{
	char *p;

	GetModuleFileName(hModule, lpFilename, nSize);
	p = strrchr(lpFilename, '\\');
	if (p){
		p++;
	} else {
		p = lpFilename;
	}
	strcpy(p, DRIVER_NAME);
	
}

BOOL CheckExecutingSameProcess()
{
	
	if (hProcessEv != INVALID_HANDLE_VALUE)
		CloseHandle(hProcessEv);
		
	hProcessEv = CreateEvent(NULL, TRUE, FALSE, PROCESS_EVENT_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS){
		CloseHandle(hProcessEv);
		return TRUE;
	}
	return FALSE;
}
