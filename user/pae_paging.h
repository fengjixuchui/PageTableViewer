#ifndef __PAE_PAGING_H__
#define __PAE_PAGING_H__

#include "simple_list.h"

typedef struct _PAGE_DIRECTORY_POINTER_TABLE_REGISTER {
	unsigned pad0	:3;
	unsigned PWT	:1;
	unsigned PCD	:1;
	unsigned Base	:27;
} PAGE_DIRECTORY_POINTER_TABLE_REGISTER;

typedef struct _PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY {
	union{
		struct{
			unsigned P	:1;
			unsigned rsvd1	:2;
			unsigned PWT	:1;
			unsigned PCD	:1;
			unsigned revd5	:4;
			unsigned Ign	:3;
			unsigned __int64 Addr	:24;
			unsigned revd36	:28;
		};
		char size[8];
		unsigned __int64 raw;
	};
} PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY;

typedef struct _PAE_PAGE_DIRECTORY_ENTRY {
	union{
		struct{
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
			unsigned __int64 Addr	:24;
			unsigned revd36	:27;
			unsigned XD	:1;
		};
		char size[8];
		unsigned __int64 raw;
	};
} PAE_PAGE_DIRECTORY_ENTRY;

typedef struct _PAE_PAGE_TABLE_ENTRY {
	union{
		struct{
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
			unsigned __int64 Addr	:24;
			unsigned revd36	:27;
			unsigned XD	:1;
		};
		char size[8];
		unsigned __int64 raw;
	};
} PAE_PAGE_TABLE_ENTRY;


void* GetPdptBase(HANDLE hFile);
PAGE_DIRECTORY_POINTER_TABLE_REGISTER GetPaePDPTR(HANDLE hFile);
PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* GetPaePDPTEs(HANDLE hFile, void* pdptr);
PAE_PAGE_DIRECTORY_ENTRY** GetPaePDEs(HANDLE hFile, void* pdptr);
PAE_PAGE_TABLE_ENTRY*** GetPaePTEs(HANDLE hFile, void* pdptr);



typedef struct _PAE_PAGING_STRUCTURE{
	struct _PAE_PAGING_STRUCTURE* next;
	struct _PAE_PAGING_STRUCTURE* prev;
	void* pdptr;
	PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* pPDPTE;	// PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY pPDPTE[4];
	PAE_PAGE_DIRECTORY_ENTRY** ppPDE;	// PAGE_DIRECTORY_ENTRY pPDE[4][512]
	PAE_PAGE_TABLE_ENTRY*** pppPTE;	// PAGE_TABLE_ENTRY ppPTE[4][512][512]
} PAE_PAGING_STRUCTURE;


PAE_PAGING_STRUCTURE* GetPdptrStructure(void* pdptr);
void DelPdptrStructure(void* pdptr);

#endif /* __PAE_PAGING_H__ */
