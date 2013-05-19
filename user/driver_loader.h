#ifndef _DRIVER_LOADER_H_
#define _DRIVER_LOADER_H_

int LoadDriver(char *szDriverName, char* pDriverPath);
int UnloadDriver(char *szDriverName);

HANDLE CreateDriverFile(char* pDeviceName, char* pDriverName, char* pDriverPath);	// if (pDriverName == NULL || pDriverPath == NULL) && driver file was not opened then not try to load driver.
void CloseDriverFile(HANDLE hFile, char* pDriverName);	// if pDriverName == NULL then not unload driver.


#endif /* _DRIVER_LOADER_H_ */
