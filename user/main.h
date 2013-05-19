#ifndef __MAIN_H__
#define __MAIN_H__

#define DEVICE_NAME "PAGE_TABLE_VIEWER"
#define DRIVER_NAME "PAGE_TABLE_VIEWER.sys"

#define PROCESS_EVENT_NAME "PAGE_TABLE_VIEWER_IS_EXECUTING"

void GetDriverPath(HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
BOOL CheckExecutingSameProcess();	// if the same process is already executed, then return true

#endif /* __MAIN_H__ */
