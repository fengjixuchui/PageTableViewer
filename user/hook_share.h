#ifndef __HOOK_SHARE_H__
#define __HOOK_SHARE_H__

#define SHARED_DATA_MAP_NAME "PageTableViewer_SharedFileMap_PID:%d"
#define REQUEST_EVENT_NAME "PageTableViewer_RequestEvent_PID:%d"
#define HANDLER_STANDBY_EVENT_NAME "PageTableViewer_HandlerStandbyEvent_PID:%d"

// nRequest
#define PTV_REQUEST_NULL 0	// caller should not use this
#define PTV_REQUEST_QUIT 1
#define PTV_REQUEST_NOP 2
#define PTV_REQUEST_GET_CR3 3

typedef struct {
	// control data
	unsigned nRequest;
	// process data
	void* cr3;
} SHARED_DATA;

// don't call these func at the same time
const char* GetSharedDataMapName(int pid);	// if pid == -1 then get own name
const char* GetRequestEventName(int pid);
const char* GetHandlerStandbyEventName(int pid);

HANDLE MapSharedData(const char* pName, void** pData, unsigned nSize);
void UnmapSharedData(HANDLE hMap, void* pData);

#endif /* __HOOK_SHARE_H__ */
