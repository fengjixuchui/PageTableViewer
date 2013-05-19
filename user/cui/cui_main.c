#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "../main.h"
#include "../driver_loader.h"
#include "../kernel_interface.h"
#include "cui_main.h"

HANDLE hDriverFile;
static HANDLE hProcessEv = INVALID_HANDLE_VALUE;

int main(int argc, char** argv)
{
	char szDriverPath[0x400];
	
	GetDriverPath(NULL, szDriverPath, sizeof(szDriverPath) / sizeof(char));
	hDriverFile = CreateDriverFile(DEVICE_NAME, CheckExecutingSameProcess() ? NULL : DRIVER_NAME, szDriverPath);
	
	SwitchExecutionMode(argc, argv);
	
	CloseDriverFile(hDriverFile, CheckExecutingSameProcess() ? NULL : DRIVER_NAME);
	
	if (hProcessEv != INVALID_HANDLE_VALUE)
		CloseHandle(hProcessEv);
	
	return 0;
}

void GetDriverPath(HMODULE hModule, LPTSTR lpFilename, DWORD nSize)
{
	char *p;

	GetModuleFileName(hModule, lpFilename, nSize);
	p = strrchr(lpFilename, '\\');
	if (p){
		p++;
	} else {
		p = lpFilename;
	}
	strcpy(p, DRIVER_NAME);
	
}

BOOL CheckExecutingSameProcess()
{
	
	if (hProcessEv != INVALID_HANDLE_VALUE)
		CloseHandle(hProcessEv);
		
	hProcessEv = CreateEvent(NULL, TRUE, FALSE, PROCESS_EVENT_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS){
		CloseHandle(hProcessEv);
		return TRUE;
	}
	return FALSE;
}

BOOL CheckOption(int argc, char** argv, char* option)
{
	int n;
	
	for(n = 0; n < argc; n++){
		if (!strcmp(argv[n], option))
			return TRUE;
	}
	return FALSE;
}

BOOL GetOptionString(int argc, char** argv, char* option, char* buf, int size)
{
	int n;

	for(n = 0; n < (argc - 1); n++){
		if (!strcmp(argv[n], option)){
			if (size >= 0)
				strncpy(buf, argv[n+1], size);
			else
				strcpy(buf, argv[n+1]);
		
			return TRUE;
		}
	}
	return FALSE;
}

void DataDumpHandler(int argc, char** argv, void func())
{

	char str[0x400];
	int n;
	
	unsigned s_adr, size;
	BOOL bLoc;
	char* pbuf;
	
	s_adr	= GetOptionString(argc, argv, "-adr", str, sizeof(str) / sizeof(char)) ? strtoul(str, NULL, 16) : ((func == CopyKernelMemory) ? 0x00400000 : 0);
	size	= GetOptionString(argc, argv, "-size", str, sizeof(str) / sizeof(char)) ? strtoul(str, NULL, 16) : 0x40;
	bLoc	= !(CheckOption(argc, argv, "-LOC") || CheckOption(argc, argv, "-L"));
	
	pbuf = malloc(size);
	
	if (func == CopyKernelMemory)
		CopyKernelMemory(hDriverFile, pbuf, (void*)s_adr, size);
	else if (func == CopyPhysicalMemory)
		CopyPhysicalMemory(hDriverFile, pbuf, (LONGLONG)s_adr, size);
	
	for (n = 0; n < size; n++){
		if (!(n % 0x10)){
			printf("\r\n");
			if (bLoc)
				printf("%08x: ", s_adr + n);
		}
		printf("%02x ", (unsigned char)(pbuf[n]));
	}
	
	free(pbuf);
}

void VirtualDataDumpHandler(int argc, char** argv)
{
	DataDumpHandler(argc, argv, CopyKernelMemory);
}

void PhysicalDataDumpHandler(int argc, char** argv)
{
	DataDumpHandler(argc, argv, CopyPhysicalMemory);
}

// Configuration
extern int nPdeHead;
extern int nPdeTail;
extern int nPteHead;
extern int nPteTail;
extern int nPaePdpteHead;
extern int nPaePdpteTail;
extern int nPaePdeHead;
extern int nPaePdeTail;
extern int nPaePteHead;
extern int nPaePteTail;
extern BOOL bPrintNonP;

void PagingModeHandler(int argc, char** argv)
{
	int pid;
	
	if (CheckOption(argc, argv, "-pid")){
		char str[0x100];
		GetOptionString(argc, argv, "-pid", str, sizeof(str) / sizeof(char));
		pid = strtoul(str, NULL, 10);
	} else{
		pid = GetCurrentProcessId();
	}
	
	if (CheckOption(argc, argv, "-head") || CheckOption(argc, argv, "-tail") || CheckOption(argc, argv, "-size")){
		char str[0x100];
		BOOL bPAE = (((unsigned)ReadCR4(hDriverFile) >> 5) & 1) ? TRUE : FALSE;
		unsigned head = 0, tail = 0xffffffff;
		
		if (CheckOption(argc, argv, "-head")){
			GetOptionString(argc, argv, "-head", str, sizeof(str) / sizeof(char));
			head = strtoul(str, NULL, 16);
		}
		if (CheckOption(argc, argv, "-tail")){
			GetOptionString(argc, argv, "-tail", str, sizeof(str) / sizeof(char));
			tail = strtoul(str, NULL, 16);
		}
		else if (CheckOption(argc, argv, "-size")){
			int size;
			GetOptionString(argc, argv, "-size", str, sizeof(str) / sizeof(char));
			size = strtoul(str, NULL, 16);
			if (head + size > head)
				tail = head + size;
		}
		
		if (!bPAE){
			nPdeHead = head / (1 << 22);
			nPdeTail = tail / (1 << 22);
			if (nPdeHead == nPdeTail){
				nPteHead = (head % (1 << 22)) / (1 << 12);
				nPteTail = (tail % (1 << 22)) / (1 << 12);
			}
		} else{
			nPaePdpteHead = head / (1 << 30);
			nPaePdpteTail = tail / (1 << 30);
			if (nPaePdpteHead == nPaePdpteTail){
				nPaePdeHead = (head % (1 << 30)) / (1 << 21);
				nPaePdeTail = (tail % (1 << 30)) / (1 << 21);
				if (nPaePdeHead == nPaePdeTail){
					nPaePteHead = (head % (1 << 21)) / (1 << 12);
					nPaePteTail = (tail % (1 << 21)) / (1 << 12);
				}
			}
		}
	}
	
	bPrintNonP = !CheckOption(argc, argv, "-P");
	
	
	PrintPagingTree(pid);
}

char szHelp[] = 
	"[pg] Print Paging Table\n"
	" -pid\n"
	" -P\n"
	" -head\n"
	" -tail\n"
	" -size\n"
	"[vdd] Virtual Data Dump\n"
	" -adr\n"
	" -size\n"
	" -L\n"
	"[pdd] Physical Data Dump\n"
	" same as vdd\n"
;

void SwitchExecutionMode(int argc, char** argv)
{
	if (CheckOption(argc, argv, "vdd")){
		VirtualDataDumpHandler(argc, argv);
	}
	else if (CheckOption(argc, argv, "pdd")){
		PhysicalDataDumpHandler(argc, argv);
	}
	else if (CheckOption(argc, argv, "pg")){
		PagingModeHandler(argc, argv);
	}
	else if (CheckOption(argc, argv, "help") || CheckOption(argc, argv, "-help")){
		printf("%s", szHelp);
	}
	else {
		PagingModeHandler(argc, argv);
	}
}

