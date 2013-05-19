#include <ntddk.h>
#include "func.h"


void CopyPhysMemory(PVOID Destination, PHYSICAL_ADDRESS Source, SIZE_T Length)
{
	void* virt;

	virt = MmMapIoSpace(Source, Length, FALSE);
	if (!virt)
		return;
	RtlCopyMemory(Destination, virt, Length);
	MmUnmapIoSpace(virt, Length);
	
}

volatile void CheckAccessLatency(void* addr, unsigned* buf, int size)
{
	unsigned __int64 a, b, diff;
	unsigned char t;
	
	a = __rdtsc();
	a = __rdtsc();
	b = __rdtsc();
	diff = b - a;
	
	if (size >= 1 * sizeof(unsigned)){	// Data
		a = __rdtsc();
		t = *(unsigned char*)addr;
		b = __rdtsc();
		buf[0] = (b - a) - diff;
	}
	
	if (size >= 2 * sizeof(unsigned)){	// Instruction
		*(unsigned char*)addr = 0xc3;	// RET
		a = __rdtsc();
		__asm call addr
		b = __rdtsc();
		buf[1] = (b - a) - diff;
		*(unsigned char*)addr = t;
	}
	
	if (size >= 3 * sizeof(unsigned)){
		buf[2] = diff;
	}
}
