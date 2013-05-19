#include <windows.h>
#include <stdio.h>
#include "driver_loader.h"


int LoadDriver(char *szDriverName, char* pDriverPath)
{
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	int ret;
	
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		return -1;
	
	hService = OpenService(hSCManager, szDriverName, SERVICE_ALL_ACCESS);	// stop driver if it's already opened
	if (hService){
		SERVICE_STATUS	serviceStatus;
		ret = ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);
		ret = DeleteService(hService);
		CloseServiceHandle(hService);
	}
	
	hService = CreateService(
		hSCManager,
		szDriverName,
		szDriverName,
		SERVICE_ALL_ACCESS,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		pDriverPath,
		NULL,NULL,NULL,NULL,NULL
	);
		
	 if (hService) {
		ret = StartService(hService, 0, NULL);
		if (!ret)
			ret = -2;
		if (!CloseServiceHandle(hService))
			ret = -3;
	} else {
		ret = -4;
	}
	
	if (!CloseServiceHandle(hSCManager))
		ret = -5;
	
	return ret;
}

int UnloadDriver(char *szDriverName)
{
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	int ret;

	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		ret = -1;
		
	hService = OpenService(hSCManager, szDriverName, SERVICE_ALL_ACCESS);
	
	if (hService) {
		SERVICE_STATUS	serviceStatus;
	    ret = ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);
		if (ret){
	        ret = DeleteService(hService);
			if (!ret)
				ret = -2;
		}
	    if (!CloseServiceHandle(hService))
			ret = -3;
	}else{
		ret = -4;
	}
	
	if (!CloseServiceHandle(hSCManager))
		ret = -5;
	
	return ret;
}


HANDLE CreateDriverFile(char* pDeviceName, char* pDriverName, char* pDriverPath)
{
	char szBuf[0x800] ={0};
	HANDLE hFile;

	sprintf(szBuf,"\\\\.\\%s", pDeviceName);
	
	hFile = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);	// if driver is already loaded, not re-load it
	if (hFile != INVALID_HANDLE_VALUE)
		return hFile;
		
	if (!pDriverName || !pDriverPath)
		return (HANDLE)(-1);
	
	LoadDriver(pDriverName, pDriverPath);
	
	hFile = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE){
		printf("CreateFile failed.\n");
		return (HANDLE)(-1);
	}
	
	return hFile;
}

void CloseDriverFile(HANDLE hFile, char* pDriverName)
{

	CloseHandle(hFile);
	
	if (!pDriverName)
		return;

	UnloadDriver(pDriverName);
	
}
