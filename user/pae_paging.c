#include <windows.h>
#include "kernel_interface.h"
#include "pae_paging.h"


static PAE_PAGING_STRUCTURE* pFirstPS = NULL;

PAE_PAGING_STRUCTURE* GetPdptrStructure(void* pdptr)
{
	return (PAE_PAGING_STRUCTURE*)GetListItem(pdptr, &pFirstPS, sizeof(PAE_PAGING_STRUCTURE));
}

void DelPdptrStructure(void* pdptr)
{
	DeleteListItem(pdptr, pFirstPS, sizeof(PAE_PAGING_STRUCTURE));
}


void* GetPdptBase(HANDLE hFile)
{
	return (void*)(unsigned)(ReadCR3(hFile) & 0xffffffe0);
}

PAGE_DIRECTORY_POINTER_TABLE_REGISTER GetPaePDPTR(HANDLE hFile)
{
	PAGE_DIRECTORY_POINTER_TABLE_REGISTER* p;
	unsigned r;
	
	r = (unsigned)ReadCR3(hFile);
	p = &r;
	return *p;
}

PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* GetPaePDPTEs(HANDLE hFile, void* pdptr)
{
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(pdptr ? pdptr : GetPdptBase(hFile));
	
	if (!ps->pPDPTE)
		ps->pPDPTE = VirtualAlloc(NULL, 8 * 4, MEM_COMMIT, PAGE_READWRITE);
	
	CopyPhysicalMemory(hFile, ps->pPDPTE, pdptr ? pdptr : GetPdptBase(hFile), 8 * 4);
	
	return ps->pPDPTE;
}

PAE_PAGE_DIRECTORY_ENTRY** GetPaePDEs(HANDLE hFile, void* pdptr)
{
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(pdptr ? pdptr : GetPdptBase(hFile));
	int i;
	
	if (!ps->ppPDE)
		ps->ppPDE = VirtualAlloc(NULL, sizeof(PAE_PAGE_DIRECTORY_ENTRY*) * 4, MEM_COMMIT, PAGE_READWRITE);
	
	for (i = 0; i < 4; i++){
		if (!ps->pPDPTE[i].P)
			continue;
		if (!ps->ppPDE[i])
			ps->ppPDE[i] = VirtualAlloc(NULL, 8 * 512, MEM_COMMIT, PAGE_READWRITE);
		CopyPhysicalMemory(hFile, ps->ppPDE[i], ps->pPDPTE[i].Addr << 12, 8 * 512);
	}
	
	return ps->ppPDE;
}

PAE_PAGE_TABLE_ENTRY*** GetPaePTEs(HANDLE hFile, void* pdptr)
{
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(pdptr ? pdptr : GetPdptBase(hFile));
	int i, j;
	
	if (!ps->pppPTE){
		ps->pppPTE = VirtualAlloc(NULL, sizeof(PAE_PAGE_TABLE_ENTRY**) * 4, MEM_COMMIT, PAGE_READWRITE);
		for (i = 0; i < 4; i++){
			ps->pppPTE[i] = VirtualAlloc(NULL, sizeof(PAE_PAGE_TABLE_ENTRY*) * 512, MEM_COMMIT, PAGE_READWRITE);
		}
	}
	
	for (i = 0; i < 4; i++){
		if (!ps->pPDPTE[i].P)
			continue;
		for (j = 0; j < 512; j++){
			if (!ps->ppPDE[i][j].P || ps->ppPDE[i][j].PS)
				continue;
			if (!ps->pppPTE[i][j])
				ps->pppPTE[i][j] = VirtualAlloc(NULL, 8 * 512, MEM_COMMIT, PAGE_READWRITE);
			CopyPhysicalMemory(hFile, ps->pppPTE[i][j], ps->ppPDE[i][j].Addr << 12, 8 * 512);
		}
	}
	
	return ps->pppPTE;
}
