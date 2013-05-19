#ifndef __HOOK_ANLGER_H__
#define __HOOK_ANLGER_H__

#define HOOK_DLL_NAME "hook_ptviewer.dll"

void SetHook(DWORD pid, char* pDllPath, BOOL bAutoFree);
// if pDllPath is NULL then use HOOK_DLL_NAME
// if bAutoFree == FALSE, injected dll have to be freelibrary manually when DllMain:DLL_PROCESS_ATTACH is finished

#endif /* __HOOK_ANLGER_H__ */
