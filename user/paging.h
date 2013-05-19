#ifndef __PAGING_H__
#define __PAGING_H__

#include "simple_list.h"

typedef struct _PAGE_DIRECTORY_BASE_REGISTER {
	unsigned pad0	:3;
	unsigned PWT	:1;
	unsigned PCD	:1;
	unsigned pad5	:7;
	unsigned Base	:20;
} PAGE_DIRECTORY_BASE_REGISTER;

typedef struct _PAGE_DIRECTORY_ENTRY {
	unsigned P	:1;
	unsigned RW	:1;
	unsigned US	:1;
	unsigned PWT	:1;
	unsigned PCD	:1;
	unsigned A	:1;
	unsigned D	:1;
	unsigned PS	:1;
	unsigned G	:1;
	unsigned Avail	:3;
	unsigned Addr	:20;
} PAGE_DIRECTORY_ENTRY;

typedef struct _PAGE_TABLE_ENTRY {
	unsigned P	:1;
	unsigned RW	:1;
	unsigned US	:1;
	unsigned PWT	:1;
	unsigned PCD	:1;
	unsigned A	:1;
	unsigned D	:1;
	unsigned PAT	:1;
	unsigned G	:1;
	unsigned Avail	:3;
	unsigned Addr	:20;
} PAGE_TABLE_ENTRY;


void* GetPdBase(HANDLE hFile);
PAGE_DIRECTORY_BASE_REGISTER GetPDBR(HANDLE hFile);
PAGE_DIRECTORY_ENTRY* GetPDEs(HANDLE hFile, void* pdbr);
PAGE_TABLE_ENTRY** GetPTEs(HANDLE hFile, void* pdbr);



typedef struct _PAGING_STRUCTURE{
	struct _PAGING_STRUCTURE* next;
	struct _PAGING_STRUCTURE* prev;
	void* pdbr;
	PAGE_DIRECTORY_ENTRY* pPDE;	// PAGE_DIRECTORY_ENTRY pPDE[1024]
	PAGE_TABLE_ENTRY** ppPTE;	// PAGE_TABLE_ENTRY ppPTE[1024][1024]
} PAGING_STRUCTURE;


PAGING_STRUCTURE* GetPdbrStructure(void* pdbr);
void DelPdbrStructure(void* pdbr);

#endif /* __PAGING_H__ */
