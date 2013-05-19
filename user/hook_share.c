#include <windows.h>
#include <stdio.h>
#include "hook_share.h"

static char szName[0x80];

const char* GetSharedDataMapName(int pid)
{
	
	sprintf(szName, SHARED_DATA_MAP_NAME, (pid != -1) ? pid : GetCurrentProcessId());
	
	return szName;
}

const char* GetRequestEventName(int pid)
{
	
	sprintf(szName, REQUEST_EVENT_NAME, (pid != -1) ? pid : GetCurrentProcessId());
	
	return szName;
}

const char* GetHandlerStandbyEventName(int pid)
{
	
	sprintf(szName, HANDLER_STANDBY_EVENT_NAME, (pid != -1) ? pid : GetCurrentProcessId());
	
	return szName;
}


HANDLE MapSharedData(const char* pName, void** pData, unsigned nSize)
{
	HANDLE hMap;
	
	hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nSize, pName);
	if (!hMap) return -1;
	*pData = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	
	return hMap;
}

void UnmapSharedData(HANDLE hMap, void* pData)
{

	if (pData)
		UnmapViewOfFile(pData);
	if (hMap != INVALID_HANDLE_VALUE)
		CloseHandle(hMap);

}
