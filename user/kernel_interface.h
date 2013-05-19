#ifndef __KERNEL_INTERFACE_H__
#define __KERNEL_INTERFACE_H__

void CopyKernelMemory(HANDLE hFile, void* dest, void* source, unsigned size);
void CopyPhysicalMemory(HANDLE hFile, void* dest, LONGLONG source, unsigned size);	// dest: virtual address, source: physical address
LONGLONG ReadCR0(HANDLE hFile);
LONGLONG ReadCR3(HANDLE hFile);
LONGLONG ReadCR4(HANDLE hFile);
LONGLONG ReadMSR(HANDLE hFile, unsigned reg);
void WriteMSR(HANDLE hFile, unsigned reg, LONGLONG value);

void DoWbinvd(HANDLE hFile);
void DoInvlpg(HANDLE hFile, void* addr);
void CheckAccessLatency(HANDLE hFile, void* addr, unsigned* buf, int size);

#endif /* __KERNEL_INTERFACE_H__ */
