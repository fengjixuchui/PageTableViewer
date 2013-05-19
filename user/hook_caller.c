#include <windows.h>
#include <tlhelp32.h>
#include "hook_angler.h"
#include "hook_share.h"
#include "hook_caller.h"


SHARED_DATA* CallRequestHandler(DWORD pid, unsigned nRequest, int nTimeout)	// don't call this func in parallel
{
	static SHARED_DATA* pSharedData = NULL;
	static HANDLE hMap = INVALID_HANDLE_VALUE;
	static HANDLE hReqEv = INVALID_HANDLE_VALUE;
	HANDLE hStandbyEv;

	static DWORD nLastPid;
	
	DWORD r;
	
	SetLastError(0);
	hStandbyEv = CreateEvent(NULL, TRUE, FALSE, GetHandlerStandbyEventName(pid));	// same pid doesn't always mean same process
	if (GetLastError() != ERROR_ALREADY_EXISTS){
		if (nTimeout == 0){
			CloseHandle(hStandbyEv);
			return NULL;
		}
		SetHook(pid, NULL, FALSE);
	}
	r = WaitForSingleObject(hStandbyEv, nTimeout);
	CloseHandle(hStandbyEv);
	if (r == WAIT_TIMEOUT)
		return NULL;
		
	if ((pid != nLastPid) || !pSharedData || (hMap == INVALID_HANDLE_VALUE) || (hReqEv == INVALID_HANDLE_VALUE)){
	
		nLastPid = pid;
		if (pSharedData || (hMap != INVALID_HANDLE_VALUE)){
			UnmapSharedData(hMap, pSharedData);
			pSharedData = NULL;
			hMap = INVALID_HANDLE_VALUE;
		}
		if (hReqEv != INVALID_HANDLE_VALUE){
			CloseHandle(hReqEv);
			hReqEv = INVALID_HANDLE_VALUE;
		}
		
		hMap = MapSharedData(GetSharedDataMapName(pid), &pSharedData, sizeof(SHARED_DATA));
		hReqEv = CreateEvent(NULL, FALSE, FALSE, GetRequestEventName(pid));
		
		if (!pSharedData || (hReqEv == INVALID_HANDLE_VALUE))
			return NULL;
	}
	
	pSharedData->nRequest = nRequest;
	SetEvent(hReqEv);
	Sleep(0);
	WaitForSingleObject(hReqEv, INFINITE);
	
	return pSharedData;
}


void* GetCR3ofPid(DWORD pid)
{
	SHARED_DATA* pData;

	pData = CallRequestHandler(pid, PTV_REQUEST_GET_CR3, 500);
	
	return (pData ? pData->cr3 : NULL);
}


int GetProcessIdLists(DWORD* pList, int nNum)
{
	HANDLE hSnp;
	PROCESSENTRY32 pe;
	BOOL b;
	int i = 0;
	
	hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnp == INVALID_HANDLE_VALUE)
		return 0;
	pe.dwSize = sizeof(PROCESSENTRY32);
	
	b = Process32First(hSnp, &pe);
	while(i < nNum){
		if (!b)
			break;
		pList[i++] = pe.th32ProcessID;
		b = Process32Next(hSnp, &pe);
	}
	
	CloseHandle(hSnp);
	
	return i;
}

void FreeRemoteHandler(int pid)
{
	if (pid != -1){
		CallRequestHandler(pid, PTV_REQUEST_QUIT, 0);
		return;
	} else{
		HANDLE hSnp;
		PROCESSENTRY32 pe;
		BOOL b;
		
		pid = GetCurrentProcessId();
		
		hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnp == INVALID_HANDLE_VALUE)
			return;
		pe.dwSize = sizeof(PROCESSENTRY32);
		
		b = Process32First(hSnp, &pe);
		while(1){
			if (!b)
				break;
			if (pe.th32ProcessID != pid)
				CallRequestHandler(pe.th32ProcessID, PTV_REQUEST_QUIT, 0);
			b = Process32Next(hSnp, &pe);
		}
		
		CloseHandle(hSnp);
	}
}
