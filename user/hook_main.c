#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <tlhelp32.h>
#include <winbase.h>
#include "hook_main.h"
#include "hook_share.h"
#include "driver_loader.h"

SHARED_DATA* pSharedData = NULL;


int GetThreadIdLists(DWORD* pList, int nNum)
{
	HANDLE hSnp;
	THREADENTRY32 te;
	BOOL b;
	DWORD pid = GetCurrentProcessId();
	int i = 0;
	
	hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hSnp == INVALID_HANDLE_VALUE)
		return 0;
	te.dwSize = sizeof(THREADENTRY32);
	
	b = Thread32First(hSnp, &te);
	while(i < nNum){
		if (!b)
			break;
		if (te.th32OwnerProcessID == pid)
			pList[i++] = te.th32ThreadID;
		b = Thread32Next(hSnp, &te);
	}
	
	CloseHandle(hSnp);
	
	return i;
}


DWORD RequestHandler(HINSTANCE hinstDLL)
{
	HANDLE hMap, hReqEv, hStandbyEv;
	HANDLE hList[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
	int c = -1;
	HANDLE hDriverFile;

	hDriverFile = CreateDriverFile(DEVICE_NAME, NULL, NULL);
	
	hMap = MapSharedData(GetSharedDataMapName(-1), &pSharedData, sizeof(SHARED_DATA));
	
	hReqEv = CreateEvent(NULL, FALSE, FALSE, GetRequestEventName(-1));
	hList[1] = hReqEv;
	hStandbyEv = CreateEvent(NULL, TRUE, FALSE, GetHandlerStandbyEventName(-1));
	
	SignalObjectAndWait(hStandbyEv, hReqEv, INFINITE, FALSE);
	while(1){
		if (pSharedData->nRequest == PTV_REQUEST_QUIT){
			break;
		}
		switch(pSharedData->nRequest){
			case PTV_REQUEST_GET_CR3:
				pSharedData->cr3 = (unsigned)ReadCR3(hDriverFile);
				break;
			default:
				break;
		}
		
		if (c == 0 || c == -1){	// thread
			DWORD nList[2];
			DWORD tid;
			
			if (GetThreadIdLists(nList, 2) > 1){	
				tid = (nList[0] != GetCurrentThreadId()) ? nList[0] : nList[1];
				if (hList[0] != INVALID_HANDLE_VALUE)
					CloseHandle(hList[0]);
				hList[0] = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
			} else{
				break;
			}
		}

		pSharedData->nRequest = 0;
		SetEvent(hReqEv);
		Sleep(0);

		c = WaitForMultipleObjects(2, hList, FALSE, INFINITE) - WAIT_OBJECT_0;
	}
	
	SetEvent(hReqEv);
	
	CloseHandle(hStandbyEv);
	CloseHandle(hReqEv);
	UnmapSharedData(hMap, pSharedData);
	
	if (hList[0] != INVALID_HANDLE_VALUE)
		CloseHandle(hList[0]);
		
	CloseDriverFile(hDriverFile, NULL);
	
	if (hinstDLL)
		FreeLibraryAndExitThread(hinstDLL, 0);
	
	return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	DWORD tid;
	
	switch (fdwReason){
		case DLL_PROCESS_ATTACH:
			CloseHandle(CreateThread(NULL, 0, RequestHandler, hinstDLL, 0, &tid));
			break;
		default:
			break;
	}
	
	return TRUE;
}
