#ifndef __GUI_VIEWER_H__
#define __GUI_VIEWER_H__

#define PTE_FORMAT "%04d: %08x %1d %s %s %s %s %s %s %s %s %s"
#define PDE_FORMAT PTE_FORMAT
#define CR3_FORMAT "%08x %s %s <PID:%5d>"
#define PAE_PDPTE_FORMAT "%01d: %09x %1d %s %s %s"
#define PAE_PTE_FORMAT "%03d: %09x %s %1d %s %s %s %s %s %s %s %s %s"
#define PAE_PDE_FORMAT PAE_PTE_FORMAT

HWND CreatePTViewerWindow(HINSTANCE hInstance);

#include <commctrl.h>

#include "paging.h"

void ComposePteItem(HTREEITEM htPDE, const PAGE_TABLE_ENTRY* pPTE, const PAGE_DIRECTORY_ENTRY* pPDE);
DWORD WINAPI ComposePteItems(void* pdbr);
DWORD WINAPI ComposePdeItems(void* pdbr);
void* ComposePdbrItem();

#include "pae_paging.h"

void ComposePaePteItem(HTREEITEM htPDE, const PAE_PAGE_TABLE_ENTRY* pPPTE);
DWORD WINAPI ComposePaePteItems(void* pdptr);
void ComposePaePdeItem(HTREEITEM htPDPTE, const PAE_PAGE_DIRECTORY_ENTRY* pPPDE, HTREEITEM* phtPDE);
DWORD WINAPI ComposePaePdeItems(void* pdptr);
DWORD WINAPI ComposePaePdpteItems(void* pdptr);
void* ComposePaePdptrItem();


#include "simple_list.h"

typedef struct _PAGING_TREE_ITEM_HANDLERS {
	struct _PAE_PAGING_STRUCTURE* next;
	struct _PAE_PAGING_STRUCTURE* prev;
	void* pdbr;
	HTREEITEM htCR3;
	HTREEITEM htPDE[1024];
} PAGING_TREE_ITEM_HANDLERS;

typedef struct _PAE_PAGING_TREE_ITEM_HANDLERS {
	struct _PAE_PAGING_STRUCTURE* next;
	struct _PAE_PAGING_STRUCTURE* prev;
	void* pdptr;
	HTREEITEM htCR3;
	HTREEITEM htPDPTE[4];
	HTREEITEM htPDE[4][512];
} PAE_PAGING_TREE_ITEM_HANDLERS;

PAGING_TREE_ITEM_HANDLERS* GetPagingTreeItemHandlers(void* pdbr);
void DelPagingTreeItemHandlers(void* pdbr);
PAE_PAGING_TREE_ITEM_HANDLERS* GetPaePagingTreeItemHandlers(void* pdptr);
void DelPaePagingTreeItemHandlers(void* pdptr);


void GetWindowPos(HWND hParentWnd, HWND hClientWnd, RECT* pRect);
int GetTreeViewItemLevel(HWND hwndTV, HTREEITEM hitem);
HTREEITEM GetTreeViewItemAncestor(HWND hwndTV, HTREEITEM hitem, int back);
void* GetBaseVirtualAddressFromItem(HWND hTree, HTREEITEM hitem);
void PrintStatus(HWND hStatus, HWND hTree);
void PrintLoadingStatus(BOOL loading);
DWORD WINAPI UpdateTree(void* cr3);


//private
void ComposePagingTree();
void SetChildEntryIsChangedFlag(HWND hwndTV, HTREEITEM hitem);
void SetStatusSize(HWND hStatus, int nWidth, int nHeight);
LRESULT ViewerNotifyMessageHandler(NMHDR* pnmhdr);
void WindupViewer(HWND hTree);


//misc func
void ShowContextMenu(HWND hwnd, LPARAM lp);
void TestCache(void* addr);


#define IDC_STATUS	401

#define ENTRY_ITEM_DEFAULT	0x1
#define ENTRY_ITEM_CHANGED	0x2
#define CHILD_ENTRY_ITEM_CHANGED	0x4
#define ENTRY_ITEM_NEW	0x8
#define ENTRY_ITEM_ADDR_CHANGED	0x10

#define COLOR_DEFAULT	RGB(255, 255, 255)
#define COLOR_CHANGED	RGB(0, 255, 255)
#define COLOR_CHILD_CHANGED	RGB(255, 0, 255)
#define COLOR_NEW	RGB(255, 255, 0)
#define COLOR_ADDR_CHANGED	RGB(0, 255, 0)

#endif /* __GUI_VIEWER_H__ */
