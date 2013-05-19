#include <windows.h>
#include "../driver/ptviewer.h"
#include "kernel_interface.h"


void CopyKernelMemory(HANDLE hFile, void* dest, void* source, unsigned size)
{
	DWORD b;
	
	DeviceIoControl(hFile, IOCTL_COPY_MEMORY, &source, sizeof(source), dest, size, &b, NULL);
}

void CopyPhysicalMemory(HANDLE hFile, void* dest, LONGLONG source, unsigned size)
{
	DWORD b;
	
	DeviceIoControl(hFile, IOCTL_COPY_PHYSICAL_MEMORY, &source, sizeof(source), dest, size, &b, NULL);
}

LONGLONG ReadCR3(HANDLE hFile)
{
	LONGLONG cr3;
	DWORD b;

	DeviceIoControl(hFile, IOCTL_READ_CR3, NULL, 0, &cr3, sizeof(cr3), &b, NULL);
	return cr3;
}

LONGLONG ReadCR0(HANDLE hFile)
{
	LONGLONG r;
	DWORD b;

	DeviceIoControl(hFile, IOCTL_READ_CR0, NULL, 0, &r, sizeof(r), &b, NULL);
	return r;
}

LONGLONG ReadCR4(HANDLE hFile)
{
	LONGLONG r;
	DWORD b;

	DeviceIoControl(hFile, IOCTL_READ_CR4, NULL, 0, &r, sizeof(r), &b, NULL);
	return r;
}

LONGLONG ReadMSR(HANDLE hFile, unsigned reg)
{
	LONGLONG r;
	DWORD b;

	DeviceIoControl(hFile, IOCTL_READ_MSR, &reg, sizeof(unsigned), &r, sizeof(r), &b, NULL);
	return r;
}

void WriteMSR(HANDLE hFile, unsigned reg, LONGLONG value)
{
	unsigned r, v[3];
	DWORD b;

	v[0] = reg;
	*(LONGLONG*)(v + 1) = value;
	DeviceIoControl(hFile, IOCTL_WRITE_MSR, v, sizeof(v), &r, sizeof(r), &b, NULL);
}


void DoWbinvd(HANDLE hFile)
{
	DWORD b;

	DeviceIoControl(hFile, IOCTL_DO_WBINVD, NULL, 0, NULL, 0, &b, NULL);
}

void DoInvlpg(HANDLE hFile, void* addr)
{
	DWORD b;

	DeviceIoControl(hFile, IOCTL_DO_INVLPG, &addr, sizeof(addr), NULL, 0, &b, NULL);
}

void CheckAccessLatency(HANDLE hFile, void* addr, unsigned* buf, int size)
{
	DWORD b;

	DeviceIoControl(hFile, IOCTL_CHECK_ACCESS_LATENCY, &addr, sizeof(addr), buf, size, &b, NULL);
}
