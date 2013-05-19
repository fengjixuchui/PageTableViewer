#include <windows.h>
#include "hook_angler.h"

static char szDllPath[0x800] = {0};

char* getHookDllPath()
{
	char *p;
	
	if (szDllPath[0])
		return szDllPath;

	GetModuleFileNameA(NULL, szDllPath, sizeof(szDllPath) / sizeof(char));
	p = strrchr(szDllPath, '\\');
	if (p){
		p++;
	} else {
		p = szDllPath;
	}
	strcpy(p, HOOK_DLL_NAME);
	
	return szDllPath;
}

DWORD __SetHook(DWORD pid)
{
	HANDLE hProcess;
	void* remoteAddress;
	char *pDllPath = szDllPath;
	HANDLE hThread;
	DWORD remoteThreadId;
	HANDLE hLibModule;
	BOOL bAutoFree = (pid & 0x80000000 ? TRUE : FALSE);

	pid &= 0x7FFFFFFF;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) return -1;
	remoteAddress = VirtualAllocEx(hProcess, NULL, strlen(pDllPath), MEM_COMMIT, PAGE_READWRITE);
	if (remoteAddress){
		if (WriteProcessMemory(hProcess, remoteAddress, pDllPath, strlen(pDllPath), NULL)){
			//SetShardData();
			hThread = CreateRemoteThread(hProcess, NULL, 0, GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA"), remoteAddress, 0, &remoteThreadId);
			if (hThread){
				WaitForSingleObject(hThread, INFINITE);
				GetExitCodeThread(hThread, &hLibModule);
				CloseHandle(hThread);
		
				if (bAutoFree){
					hThread = CreateRemoteThread(hProcess, NULL, 0, GetProcAddress(GetModuleHandle("Kernel32"), "FreeLibrary"), (void*)hLibModule, 0, &remoteThreadId);
					if (hThread){
						WaitForSingleObject(hThread, INFINITE);
						CloseHandle(hThread);
					}
				}
			}
			//FreeShardData();
		}
		VirtualFreeEx(hProcess, remoteAddress, strlen(pDllPath), MEM_RELEASE);
	}
	CloseHandle(hProcess);
	
	return 0;
}


void SetHook(DWORD pid, char* pDllPath, BOOL bAutoFree)
{
	DWORD tid;
	WIN32_FIND_DATA fd;

	if (pDllPath){
		strncpy(szDllPath, pDllPath, sizeof(szDllPath) / sizeof(char));
	} else{
		if (!szDllPath[0]){
			HANDLE hFile;
			getHookDllPath();
			hFile = CreateFile(szDllPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE){
				szDllPath[0] = 0;
			} else{
				CloseHandle(hFile);
			}
		#if 0	// alternative
			char* p;
			if (!SearchPath(NULL, HOOK_DLL_NAME, NULL, sizeof(szDllPath) / sizeof(char), szDllPath, &p))
				szDllPath[0] = 0;
		#endif
		}
	}
	
	if (szDllPath[0])
		CreateThread(NULL, 0, __SetHook, pid | (bAutoFree ? 0x80000000 : 0), 0, &tid);
}
