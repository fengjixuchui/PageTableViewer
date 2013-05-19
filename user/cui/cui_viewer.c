#include <windows.h>
#include "../kernel_interface.h"
#include "../hook_caller.h"
#include "../paging.h"
#include "../pae_paging.h"
#include "cui_viewer.h"

extern HANDLE hDriverFile;

BOOL bPAE;

// Configuration
int nPdeHead = 0;
int nPdeTail = 1023;
int nPteHead = 0;
int nPteTail = 1023;
int nPaePdpteHead = 0;
int nPaePdpteTail = 3;
int nPaePdeHead = 0;
int nPaePdeTail = 511;
int nPaePteHead = 0;
int nPaePteTail = 511;
BOOL bPrintNonP = TRUE;

void PrintPTE(unsigned cr3, int pde, int pte)
{
	PAGING_STRUCTURE* ps = GetPdbrStructure(cr3);
	PAGE_TABLE_ENTRY* pPTE;
	
	if (!ps->pPDE)
		GetPDEs(hDriverFile, cr3);
	if (!ps->ppPTE)
		GetPTEs(hDriverFile, cr3);
	pPTE = ps->ppPTE[pde] + pte;
	if (!bPrintNonP && !pPTE->P)
		return;
	printf("-- ");
	printf(PTE_FORMAT,
		pte,
		pPTE->Addr << 12,
		pPTE->Avail,
		pPTE->G ? "G" : "-",
		pPTE->PAT ? "PAT" : "-",
		pPTE->D ? "D" : "-",
		pPTE->A ? "A" : "-",
		pPTE->PCD ? "PCD" : "-",
		pPTE->PWT ? "PWT" : "-",
		pPTE->US ? "US" : "-",
		pPTE->RW ? "RW" : "-",
		pPTE->P ? "P" : "-"
	);
	printf("\r\n");
}

void PrintPDE(unsigned cr3, int pde)
{
	int n;
	PAGING_STRUCTURE* ps = GetPdbrStructure(cr3);
	PAGE_DIRECTORY_ENTRY* pPDE;
	
	if (!ps->pPDE)
		GetPDEs(hDriverFile, cr3);
	pPDE = ps->pPDE + pde;
	if (!bPrintNonP && !pPDE->P)
		return;
	printf("- ");
	printf(PDE_FORMAT,
		pde,
		!pPDE->PS ? (pPDE->Addr << 12) : ((pPDE->Addr << 12) & 0xffc00000),
		pPDE->Avail,
		pPDE->G ? "G" : "-",
		pPDE->PS ? "PS" : "-",
		pPDE->D ? "D" : "-",
		pPDE->A ? "A" : "-",
		pPDE->PCD ? "PCD" : "-",
		pPDE->PWT ? "PWT" : "-",
		pPDE->US ? "US" : "-",
		pPDE->RW ? "RW" : "-",
		pPDE->P ? "P" : "-"
	);
	printf("\r\n");
	if (pPDE->P && !pPDE->PS){
		for (n = nPteHead; n <= nPteTail; n++){
			PrintPTE(cr3, pde, n);
		}
	}
}

void PrintPDBR(unsigned cr3, int pid)
{
	int n;

	printf(CR3_FORMAT, cr3 & 0xfffff000, ((cr3 >> 4) & 1) ? "PCD" : "-", ((cr3 >> 3) & 1) ? "PWT" : "-", pid);
	printf("\r\n");
	cr3 &= 0xfffff000;
	for (n = nPdeHead; n <= nPdeTail; n++){
		PrintPDE(cr3, n);
	}
}

void PrintPaePTE(unsigned cr3, int pdpte, int pde, int pte)
{
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(cr3);
	PAE_PAGE_TABLE_ENTRY* pPTE;
	
	if (!ps->pPDPTE)
		GetPaePDPTEs(hDriverFile, cr3);
	if (!ps->ppPDE)
		GetPaePDEs(hDriverFile, cr3);
	if (!ps->pppPTE)
		GetPaePTEs(hDriverFile, cr3);
	pPTE = ps->pppPTE[pdpte][pde] + pte;
	if (!bPrintNonP && !pPTE->P)
		return;
	printf("--- ");
	printf(PAE_PTE_FORMAT,
		pte,
		(unsigned)pPTE->Addr << 12,
		pPTE->XD ? "XD" : "-",
		pPTE->Avail,
		pPTE->G ? "G" : "-",
		pPTE->PAT ? "PAT" : "-",
		pPTE->D ? "D" : "-",
		pPTE->A ? "A" : "-",
		pPTE->PCD ? "PCD" : "-",
		pPTE->PWT ? "PWT" : "-",
		pPTE->US ? "US" : "-",
		pPTE->RW ? "RW" : "-",
		pPTE->P ? "P" : "-"
	);
	printf("\r\n");
}

void PrintPaePDE(unsigned cr3, int pdpte, int pde)
{
	int n;
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(cr3);
	PAE_PAGE_DIRECTORY_ENTRY* pPDE;
	
	if (!ps->pPDPTE)
		GetPaePDPTEs(hDriverFile, cr3);
	if (!ps->ppPDE)
		GetPaePDEs(hDriverFile, cr3);
	pPDE = ps->ppPDE[pdpte] + pde;
	if (!bPrintNonP && !pPDE->P)
		return;
	printf("-- ");
	printf(PAE_PDE_FORMAT,
		pde,
		(unsigned)(!pPDE->PS ? (pPDE->Addr << 12) : ((pPDE->Addr << 12) & 0xffe00000)),
		pPDE->XD ? "XD" : "-",
		pPDE->Avail,
		pPDE->G ? "G" : "-",
		pPDE->PS ? "PS" : "-",
		pPDE->D ? "D" : "-",
		pPDE->A ? "A" : "-",
		pPDE->PCD ? "PCD" : "-",
		pPDE->PWT ? "PWT" : "-",
		pPDE->US ? "US" : "-",
		pPDE->RW ? "RW" : "-",
		pPDE->P ? "P" : "-"
	);
	printf("\r\n");
	if (pPDE->P && !pPDE->PS){
		for (n = nPaePteHead; n <= nPaePteTail; n++){
			PrintPaePTE(cr3, pdpte, pde, n);
		}
	}
}

void PrintPaePDPTE(unsigned cr3, int pdpte)
{
	int n;
	PAE_PAGING_STRUCTURE* ps = GetPdptrStructure(cr3);
	PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* pPDPTE;
	
	if (!ps->pPDPTE)
		GetPaePDPTEs(hDriverFile, cr3);
	pPDPTE = ps->pPDPTE + pdpte;
	if (!bPrintNonP && !pPDPTE->P)
		return;
	printf("- ");
	printf(PAE_PDPTE_FORMAT,
		pdpte,
		(unsigned)pPDPTE->Addr << 12,
		pPDPTE->Ign,
		pPDPTE->PCD ? "PCD" : "-",
		pPDPTE->PWT ? "PWT" : "-",
		pPDPTE->P ? "P" : "-"
	);
	printf("\r\n");
	if (pPDPTE->P){
		for (n = nPaePdeHead; n <= nPaePdeTail; n++){
			PrintPaePDE(cr3, pdpte, n);
		}
	}
}

void PrintPaePDPTR(unsigned cr3, int pid)
{
	int n;

	printf(CR3_FORMAT, cr3 & 0xffffffe0, ((cr3 >> 4) & 1) ? "PCD" : "-", ((cr3 >> 3) & 1) ? "PWT" : "-", pid);
	printf("\r\n");
	cr3 &= 0xffffffe0;
	for (n = nPaePdpteHead; n <= nPaePdpteTail; n++){
		PrintPaePDPTE(cr3, n);
	}
}

void PrintPagingTree(int pid)
{
	unsigned cr3;
	
	bPAE = (((unsigned)ReadCR4(hDriverFile) >> 5) & 1) ? TRUE : FALSE;

	if (pid == -1){
		// not implemented
		return;
	} else if (pid == GetCurrentProcessId() || pid == -2){
		pid = GetCurrentProcessId();
		cr3 = (unsigned)ReadCR3(hDriverFile);
	} else{
		cr3 = GetCR3ofPid(pid);
		FreeRemoteHandler(pid);
	}
	
	if (!bPAE){
		PrintPDBR(cr3, pid);
	} else{
		PrintPaePDPTR(cr3, pid);
	}
	
}


