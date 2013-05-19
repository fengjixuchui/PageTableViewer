#include <windows.h>
#include "kernel_interface.h"
#include "paging.h"


static PAGING_STRUCTURE* pFirstPS = NULL;

PAGING_STRUCTURE* GetPdbrStructure(void* pdbr)
{
	return (PAGING_STRUCTURE*)GetListItem(pdbr, &pFirstPS, sizeof(PAGING_STRUCTURE));
}

void DelPdbrStructure(void* pdbr)
{
	DeleteListItem(pdbr, pFirstPS, sizeof(PAGING_STRUCTURE));
}


void* GetPdBase(HANDLE hFile)
{
	return (void*)(unsigned)(ReadCR3(hFile) & 0xfffff000);
}

PAGE_DIRECTORY_BASE_REGISTER GetPDBR(HANDLE hFile)
{
	PAGE_DIRECTORY_BASE_REGISTER* p;
	unsigned r;
	
	r = (unsigned)ReadCR3(hFile);
	p = &r;
	return *p;
}


PAGE_DIRECTORY_ENTRY* GetPDEs(HANDLE hFile, void* pdbr)
{
	PAGING_STRUCTURE* ps = GetPdbrStructure(pdbr ? pdbr : GetPdBase(hFile));

	if (!ps->pPDE)
		ps->pPDE = VirtualAlloc(NULL, 4 * 1024, MEM_COMMIT, PAGE_READWRITE);
		
	CopyPhysicalMemory(hFile, ps->pPDE, (LONGLONG)(pdbr ? pdbr : GetPdBase(hFile)), 4 * 1024);
	
	return ps->pPDE;
}

PAGE_TABLE_ENTRY** GetPTEs(HANDLE hFile, void* pdbr)
{
	PAGING_STRUCTURE* ps = GetPdbrStructure(pdbr ? pdbr : GetPdBase(hFile));
	int i;
	
	if (!ps->ppPTE)
		ps->ppPTE = VirtualAlloc(NULL, sizeof(PAGE_TABLE_ENTRY*) * 1024, MEM_COMMIT, PAGE_READWRITE);
	
	for (i = 0; i < 1024; i++){
		if (!ps->pPDE[i].P || ps->pPDE[i].PS)
			continue;
		if (!ps->ppPTE[i])
			ps->ppPTE[i] = VirtualAlloc(NULL, 4 * 1024, MEM_COMMIT, PAGE_READWRITE);
		CopyPhysicalMemory(hFile, ps->ppPTE[i], (LONGLONG)(ps->pPDE[i].Addr << 12), 4 * 1024);
	}
	
	return ps->ppPTE;
}
