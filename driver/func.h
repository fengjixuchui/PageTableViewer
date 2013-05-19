#ifndef __FUNC_H__
#define __FUNC_H__

void CopyPhysMemory(PVOID Destination, PHYSICAL_ADDRESS Source, SIZE_T Length);
volatile void CheckAccessLatency(void* addr, unsigned* buf, int size);

#endif /* __FUNC_H__ */
