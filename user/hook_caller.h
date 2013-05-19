#ifndef __HOOK_CALLER_H__
#define __HOOK_CALLER_H__

#include "hook_share.h"

// don't call these func in parallel
void* GetCR3ofPid(DWORD pid);
SHARED_DATA* CallRequestHandler(DWORD pid, unsigned nRequest, int nTimeout);
void FreeRemoteHandler(int pid);

int GetProcessIdLists(DWORD* pList, int nNum);

#endif /* __HOOK_CALLER_H__ */
