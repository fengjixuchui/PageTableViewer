#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0400
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "gui_viewer.h"
#include "resource.h"
#include "paging.h"
#include "pae_paging.h"
#include "kernel_interface.h"
#include "hook_gui_viewer.h"

#define NumMessageBox(i, t) {char _buf[0x100]; sprintf(_buf, "%d", (i)); MessageBox(0, _buf, (t), 0);}

extern HANDLE hDriverFile;

HWND hPagingTree = INVALID_HANDLE_VALUE;
HWND hStatus = INVALID_HANDLE_VALUE;
HWND hMenu = INVALID_HANDLE_VALUE;
HWND hContextMenu = INVALID_HANDLE_VALUE;

void* pFirstPtiHandlers = NULL;

BOOL bPAE;

void GetWindowPos(HWND hParentWnd, HWND hClientWnd, RECT* pRect)
{
	POINT pt;
	GetWindowRect(hClientWnd, pRect);
	pt.x = pRect->left; pt.y = pRect->top;
	ScreenToClient(hParentWnd, &pt);
	pRect->left = pt.x; pRect->top = pt.y;
	pt.x = pRect->right; pt.y = pRect->bottom;
	ScreenToClient(hParentWnd, &pt);
	pRect->right = pt.x; pRect->bottom = pt.y;
}

LRESULT CALLBACK PTViewerDialogProc(
	HWND hwnd,
	UINT msg,
	WPARAM wp,
	LPARAM lp
)
{
	RECT rMain, rPteTree, rStatus;

	switch(msg){
		case WM_INITDIALOG:
			GetClientRect(hwnd, &rMain);
			
			InitCommonControls();
			hPagingTree = GetDlgItem(hwnd, IDC_PTE_TREE);
			hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, "TextTmp", hwnd, IDC_STATUS);
			hMenu = GetDlgItem(hwnd, IDR_PTV_MENU);
			hContextMenu = GetSubMenu(LoadMenu((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), IDR_CONTEXT_MENU), 0);
			
			//SetWindowLong(hPagingTree, GWL_STYLE, TVS_HASBUTTONS | TVS_HASLINES | GetWindowLong(hPagingTree, GWL_STYLE));
			ComposePagingTree();
			SetStatusSize(hStatus, rMain.right, rMain.bottom);
			PrintStatus(hStatus, hPagingTree);
			
			SendMessage(hwnd, WM_SIZE, 0, rMain.right + (rMain.bottom << 16));
			break;
		case WM_CLOSE:
			WindupViewer(hwnd);
			DestroyWindow(hwnd);
			PostMessage(NULL, WM_QUIT, 0, 0);
			break;
		case WM_SIZE:
			SetStatusSize(hStatus, lp & 0xffff, (lp >> 16) & 0xffff);
			GetClientRect(hwnd, &rMain);
			GetWindowPos(hwnd, hPagingTree, &rPteTree);
			GetWindowPos(hwnd, hStatus, &rStatus);
			SetWindowPos(hPagingTree, NULL, 0, 0, rMain.right, rStatus.top, 0);
			break;
		case WM_NOTIFY:
			ViewerNotifyMessageHandler((NMHDR*)lp);
			break;
		case WM_COMMAND:
			switch(LOWORD(wp)){
				case IDM_REGISTER_VIEWER:
					CreateRegisterViewer(hwnd);
					break;
				case IDM_UPDATE_TREE:
					{
						unsigned t;
						CloseHandle(CreateThread(NULL, 0, UpdateTree, NULL, 0, &t));
					}
					break;
				case IDM_ACCESS_TEST:
					TestCache(GetBaseVirtualAddressFromItem(hPagingTree, TreeView_GetSelection(hPagingTree)));
					break;
				case IDM_INVLPG:
					DoInvlpg(hDriverFile, GetBaseVirtualAddressFromItem(hPagingTree, TreeView_GetSelection(hPagingTree)));
					break;
			}
			break;
		case WM_CONTEXTMENU:
			ShowContextMenu(hwnd, lp);
			break;
		default:
			return FALSE;
	}
	
	return TRUE;
}

HWND CreatePTViewerWindow(HINSTANCE hInstance)
{
	return CreateDialog(hInstance, IDD_PTV_DIALOG, NULL, PTViewerDialogProc);
}

void WindupViewer(HWND hTree)
{

	WindupHookViewer();
}


void ComposePteItem(HTREEITEM htPDE, const PAGE_TABLE_ENTRY* pPTE, const PAGE_DIRECTORY_ENTRY* pPDE)
{
	TVINSERTSTRUCT tvi;
	int i;
	char buf[0x100];
	
	if (pPDE)
		if (!pPDE->P)
			return;
			
	if (!htPDE)
		return;
		
	if (TreeView_GetChild(hPagingTree, htPDE)){	// update 
		unsigned t, addr;
		char text[0x100];
		tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.item.pszText = text;
		tvi.item.cchTextMax = sizeof(text);
		for (tvi.item.hItem = TreeView_GetChild(hPagingTree, htPDE); tvi.item.hItem; tvi.item.hItem = TreeView_GetNextSibling(hPagingTree, tvi.item.hItem)){
			TreeView_GetItem(hPagingTree, &(tvi.item));
			sscanf(text, PTE_FORMAT, &i, &addr, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
			sprintf(buf, PTE_FORMAT,
				i,
				pPTE[i].Addr << 12,
				pPTE[i].Avail,
				pPTE[i].G ? "G" : "-",
				pPTE[i].PAT ? "PAT" : "-",
				pPTE[i].D ? "D" : "-",
				pPTE[i].A ? "A" : "-",
				pPTE[i].PCD ? "PCD" : "-",
				pPTE[i].PWT ? "PWT" : "-",
				pPTE[i].US ? "US" : "-",
				pPTE[i].RW ? "RW" : "-",
				pPTE[i].P ? "P" : "-"
			);
			tvi.item.lParam = 0;
			if (strcmp(tvi.item.pszText, buf)){
				tvi.item.pszText = buf;
				tvi.item.lParam |= addr ? ((addr != (pPTE[i].Addr << 12)) ? ENTRY_ITEM_ADDR_CHANGED : ENTRY_ITEM_CHANGED) : ENTRY_ITEM_NEW;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				SetChildEntryIsChangedFlag(hPagingTree, tvi.item.hItem);
				tvi.item.pszText = text;
			} else{
				tvi.item.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
			}
		}
		return;
	}
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = htPDE;
	for (i = 0; i < 1024; i++){
		sprintf(buf, PTE_FORMAT,
			i,
			pPTE[i].Addr << 12,
			pPTE[i].Avail,
			pPTE[i].G ? "G" : "-",
			pPTE[i].PAT ? "PAT" : "-",
			pPTE[i].D ? "D" : "-",
			pPTE[i].A ? "A" : "-",
			pPTE[i].PCD ? "PCD" : "-",
			pPTE[i].PWT ? "PWT" : "-",
			pPTE[i].US ? "US" : "-",
			pPTE[i].RW ? "RW" : "-",
			pPTE[i].P ? "P" : "-"
		);
		tvi.item.pszText = buf;
		TreeView_InsertItem(hPagingTree, &tvi);
	}
	
}

DWORD WINAPI ComposePteItems(void* pdbr)
{
	PAGE_DIRECTORY_ENTRY* pPDE;
	PAGE_TABLE_ENTRY** ppPTE;
	PAGING_TREE_ITEM_HANDLERS* pPTIHs;
	int i;

	pPDE = GetPDEs(hDriverFile, pdbr);
	ppPTE = GetPTEs(hDriverFile, pdbr);
	pPTIHs = GetPagingTreeItemHandlers(pdbr);
	for (i = 0; i < 1024; i++){
		if (!pPDE[i].P || pPDE[i].PS)
			continue;
		ComposePteItem(pPTIHs->htPDE[i], ppPTE[i], NULL);
	}
	
	return 0;
}	

DWORD WINAPI ComposePdeItems(void* pdbr)
{
	PAGE_DIRECTORY_ENTRY* pPDE;
	PAGING_TREE_ITEM_HANDLERS* pPTIHs;
	
	TVINSERTSTRUCT tvi;
	int i;
	char buf[0x100];

	pPDE = GetPDEs(hDriverFile, pdbr);
	pPTIHs = GetPagingTreeItemHandlers(pdbr);
	if (!pPTIHs->htCR3 || pPTIHs->htCR3 == INVALID_HANDLE_VALUE)
		return 0;
		
	if (TreeView_GetChild(hPagingTree, pPTIHs->htCR3)){
		unsigned t, n, addr;
		char text[0x100];
		tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.item.pszText = text;
		tvi.item.cchTextMax = sizeof(text);
		for (n = 0; n < 1024; n++){
			tvi.item.hItem = pPTIHs->htPDE[n];
			TreeView_GetItem(hPagingTree, &(tvi.item));
			sscanf(tvi.item.pszText, PDE_FORMAT, &i, &addr, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
			sprintf(buf, PDE_FORMAT,
				i,
				!pPDE[i].PS ? (pPDE[i].Addr << 12) : ((pPDE[i].Addr << 12) & 0xffc00000),
				pPDE[i].Avail,
				pPDE[i].G ? "G" : "-",
				pPDE[i].PS ? "PS" : "-",
				pPDE[i].D ? "D" : "-",
				pPDE[i].A ? "A" : "-",
				pPDE[i].PCD ? "PCD" : "-",
				pPDE[i].PWT ? "PWT" : "-",
				pPDE[i].US ? "US" : "-",
				pPDE[i].RW ? "RW" : "-",
				pPDE[i].P ? "P" : "-"
			);
			tvi.item.lParam = 0;
			if (strcmp(tvi.item.pszText, buf)){
				tvi.item.pszText = buf;
				tvi.item.lParam |= addr ? ((addr != (pPDE[i].Addr << 12)) ? ENTRY_ITEM_ADDR_CHANGED : ENTRY_ITEM_CHANGED) : ENTRY_ITEM_NEW;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				SetChildEntryIsChangedFlag(hPagingTree, tvi.item.hItem);
				tvi.item.pszText = text;
			} else{
				tvi.item.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
			}
		}
		return 0;
	}
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = pPTIHs->htCR3;
	for (i = 0; i < 1024; i++){
		sprintf(buf, PDE_FORMAT,
			i,
			!pPDE[i].PS ? (pPDE[i].Addr << 12) : ((pPDE[i].Addr << 12) & 0xffc00000),
			pPDE[i].Avail,
			pPDE[i].G ? "G" : "-",
			pPDE[i].PS ? "PS" : "-",
			pPDE[i].D ? "D" : "-",
			pPDE[i].A ? "A" : "-",
			pPDE[i].PCD ? "PCD" : "-",
			pPDE[i].PWT ? "PWT" : "-",
			pPDE[i].US ? "US" : "-",
			pPDE[i].RW ? "RW" : "-",
			pPDE[i].P ? "P" : "-"
		);
		tvi.item.pszText = buf;
		pPTIHs->htPDE[i] = TreeView_InsertItem(hPagingTree, &tvi);
	}
	
	return 0;
}

void* ComposePdbrItem()
{
	PAGE_DIRECTORY_BASE_REGISTER pdbr;
	PAGING_TREE_ITEM_HANDLERS* pPTIHs;
	TVINSERTSTRUCT tvi;
	char buf[0x100];
	void* r;

	pdbr = GetPDBR(hDriverFile);
	r = pdbr.Base << 12;
	pPTIHs = GetPagingTreeItemHandlers(r);
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = TVI_ROOT;
	sprintf(buf, CR3_FORMAT, r, pdbr.PCD ? "PCD" : "-", pdbr.PWT ? "PWT" : "-", GetCurrentProcessId());
	tvi.item.pszText = buf;
	if (pPTIHs->htCR3){
		tvi.item.mask |= TVIF_HANDLE | TVIF_PARAM;
		tvi.item.hItem = pPTIHs->htCR3;
		tvi.item.lParam = 0;
		TreeView_SetItem(hPagingTree, &(tvi.item));
	} else{
		pPTIHs->htCR3 = TreeView_InsertItem(hPagingTree, &tvi);
	}
	
	return r;
}


void ComposePaePteItem(HTREEITEM htPDE, const PAE_PAGE_TABLE_ENTRY* pPPTE)
{
	TVINSERTSTRUCT tvi;
	int i;
	char buf[0x100];
	
	if (!htPDE)
		return;
	
	if (TreeView_GetChild(hPagingTree, htPDE)){	// update 
		unsigned t, addr;
		char text[0x100];
		tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.item.pszText = text;
		tvi.item.cchTextMax = sizeof(text);
		for (tvi.item.hItem = TreeView_GetChild(hPagingTree, htPDE); tvi.item.hItem; tvi.item.hItem = TreeView_GetNextSibling(hPagingTree, tvi.item.hItem)){
			TreeView_GetItem(hPagingTree, &(tvi.item));
			sscanf(text, PAE_PTE_FORMAT, &i, &addr, buf, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
			sprintf(buf, PAE_PTE_FORMAT,
				i,
				(unsigned)pPPTE[i].Addr << 12,
				pPPTE[i].XD ? "XD" : "-",
				pPPTE[i].Avail,
				pPPTE[i].G ? "G" : "-",
				pPPTE[i].PAT ? "PAT" : "-",
				pPPTE[i].D ? "D" : "-",
				pPPTE[i].A ? "A" : "-",
				pPPTE[i].PCD ? "PCD" : "-",
				pPPTE[i].PWT ? "PWT" : "-",
				pPPTE[i].US ? "US" : "-",
				pPPTE[i].RW ? "RW" : "-",
				pPPTE[i].P ? "P" : "-"
			);
			tvi.item.lParam = 0;
			if (strcmp(tvi.item.pszText, buf)){
				tvi.item.pszText = buf;
				tvi.item.lParam |= addr ? ((addr != ((unsigned)pPPTE[i].Addr << 12)) ? ENTRY_ITEM_ADDR_CHANGED : ENTRY_ITEM_CHANGED) : ENTRY_ITEM_NEW;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				SetChildEntryIsChangedFlag(hPagingTree, tvi.item.hItem);
				tvi.item.pszText = text;
			} else{
				tvi.item.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
			}
		}
		return;
	}
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.hParent = htPDE;
	tvi.item.mask = TVIF_TEXT;
	tvi.item.pszText = buf;
	for (i = 0; i < 512; i++){
		sprintf(tvi.item.pszText, PAE_PTE_FORMAT,
			i,
			(unsigned)pPPTE[i].Addr << 12,
			pPPTE[i].XD ? "XD" : "-",
			pPPTE[i].Avail,
			pPPTE[i].G ? "G" : "-",
			pPPTE[i].PAT ? "PAT" : "-",
			pPPTE[i].D ? "D" : "-",
			pPPTE[i].A ? "A" : "-",
			pPPTE[i].PCD ? "PCD" : "-",
			pPPTE[i].PWT ? "PWT" : "-",
			pPPTE[i].US ? "US" : "-",
			pPPTE[i].RW ? "RW" : "-",
			pPPTE[i].P ? "P" : "-"
		);
		TreeView_InsertItem(hPagingTree, &tvi);
	}
}

DWORD WINAPI ComposePaePteItems(void* pdptr)
{
	PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* pPPDPTE;
	PAE_PAGE_DIRECTORY_ENTRY** ppPPDE;
	PAE_PAGE_TABLE_ENTRY*** pppPPTE;
	PAE_PAGING_TREE_ITEM_HANDLERS* pPPTIHs;
	int i, j;

	pPPDPTE = GetPaePDPTEs(hDriverFile, pdptr);
	ppPPDE = GetPaePDEs(hDriverFile, pdptr);
	pppPPTE = GetPaePTEs(hDriverFile, pdptr);
	pPPTIHs = GetPaePagingTreeItemHandlers(pdptr);
	for (i = 0; i < 4; i++){
		if (!pPPDPTE[i].P)
			continue;
		for (j = 0; j < 512; j++){
			if (!ppPPDE[i][j].P || ppPPDE[i][j].PS)
				continue;
			ComposePaePteItem(pPPTIHs->htPDE[i][j], pppPPTE[i][j]);
		}
		
	}
	
	return 0;
}

void ComposePaePdeItem(HTREEITEM htPDPTE, const PAE_PAGE_DIRECTORY_ENTRY* pPPDE, HTREEITEM* phtPDE)
{
	TVINSERTSTRUCT tvi;
	int i;
	char buf[0x100];
	
	if (!htPDPTE)
		return;
	
	if (TreeView_GetChild(hPagingTree, htPDPTE)){
		unsigned t, addr;
		char text[0x100];
		tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.item.pszText = text;
		tvi.item.cchTextMax = sizeof(text);
		for (tvi.item.hItem = TreeView_GetChild(hPagingTree, htPDPTE); tvi.item.hItem; tvi.item.hItem = TreeView_GetNextSibling(hPagingTree, tvi.item.hItem)){
			TreeView_GetItem(hPagingTree, &(tvi.item));
			sscanf(text, PAE_PDE_FORMAT, &i, &addr, buf, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
			sprintf(buf, PAE_PDE_FORMAT,
				i,
				(unsigned)(!pPPDE[i].PS ? (pPPDE[i].Addr << 12) : ((pPPDE[i].Addr << 12) & 0xffe00000)),	//0xfffe00000
				pPPDE[i].XD ? "XD" : "-",
				pPPDE[i].Avail,
				pPPDE[i].G ? "G" : "-",
				pPPDE[i].PS ? "PS" : "-",
				pPPDE[i].D ? "D" : "-",
				pPPDE[i].A ? "A" : "-",
				pPPDE[i].PCD ? "PCD" : "-",
				pPPDE[i].PWT ? "PWT" : "-",
				pPPDE[i].US ? "US" : "-",
				pPPDE[i].RW ? "RW" : "-",
				pPPDE[i].P ? "P" : "-"
			);
			tvi.item.lParam = 0;
			if (strcmp(tvi.item.pszText, buf)){
				tvi.item.pszText = buf;
				tvi.item.lParam |= addr ? ((addr != ((unsigned)pPPDE[i].Addr << 12)) ? ENTRY_ITEM_ADDR_CHANGED : ENTRY_ITEM_CHANGED) : ENTRY_ITEM_NEW;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				SetChildEntryIsChangedFlag(hPagingTree, tvi.item.hItem);
				tvi.item.pszText = text;
			} else{
				tvi.item.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
			}
		}
		return;
	}
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = htPDPTE;
	for (i = 0; i < 512; i++){
		sprintf(buf, PAE_PDE_FORMAT,
			i,
			(unsigned)(!pPPDE[i].PS ? (pPPDE[i].Addr << 12) : ((pPPDE[i].Addr << 12) & 0xffe00000)),	//0xfffe00000
			pPPDE[i].XD ? "XD" : "-",
			pPPDE[i].Avail,
			pPPDE[i].G ? "G" : "-",
			pPPDE[i].PS ? "PS" : "-",
			pPPDE[i].D ? "D" : "-",
			pPPDE[i].A ? "A" : "-",
			pPPDE[i].PCD ? "PCD" : "-",
			pPPDE[i].PWT ? "PWT" : "-",
			pPPDE[i].US ? "US" : "-",
			pPPDE[i].RW ? "RW" : "-",
			pPPDE[i].P ? "P" : "-"
		);
		tvi.item.pszText = buf;
		phtPDE[i] = TreeView_InsertItem(hPagingTree, &tvi);
	}
	
}

DWORD WINAPI ComposePaePdeItems(void* pdptr)
{
	PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* pPPDPTE;
	PAE_PAGE_DIRECTORY_ENTRY** ppPPDE;
	PAE_PAGING_TREE_ITEM_HANDLERS* pPPTIHs;
	int i;

	pPPDPTE = GetPaePDPTEs(hDriverFile, pdptr);
	ppPPDE = GetPaePDEs(hDriverFile, pdptr);
	pPPTIHs = GetPaePagingTreeItemHandlers(pdptr);
	for (i = 0; i < 4; i++){
		if (!pPPDPTE[i].P)
			continue;
		ComposePaePdeItem(pPPTIHs->htPDPTE[i], ppPPDE[i], pPPTIHs->htPDE[i]);
	}
	
	return 0;
}

DWORD WINAPI ComposePaePdpteItems(void* pdptr)
{
	PAE_PAGE_DIRECTORY_POINTER_TABLE_ENTRY* pPPDPTE;
	PAE_PAGING_TREE_ITEM_HANDLERS* pPPTIHs;
	
	TVINSERTSTRUCT tvi;
	int i;
	char buf[0x100];

	pPPDPTE = GetPaePDPTEs(hDriverFile, pdptr);
	pPPTIHs = GetPaePagingTreeItemHandlers(pdptr);
	if (!pPPTIHs->htCR3 || pPPTIHs->htCR3 == INVALID_HANDLE_VALUE)
		return 0;
		
	if (TreeView_GetChild(hPagingTree, pPPTIHs->htCR3)){
		unsigned t, n, addr;
		char text[0x100];
		tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		tvi.item.pszText = text;
		tvi.item.cchTextMax = sizeof(text);
		for (n = 0; n < 4; n++){
			tvi.item.hItem = pPPTIHs->htPDPTE[n];
			TreeView_GetItem(hPagingTree, &(tvi.item));
			sscanf(text, PAE_PDPTE_FORMAT, &i, &addr, &t, buf, buf, buf);
			sprintf(buf, PAE_PDPTE_FORMAT,
				i,
				(unsigned)pPPDPTE[i].Addr << 12,
				pPPDPTE[i].Ign,
				pPPDPTE[i].PCD ? "PCD" : "-",
				pPPDPTE[i].PWT ? "PWT" : "-",
				pPPDPTE[i].P ? "P" : "-"
			);
			tvi.item.lParam = 0;
			if (strcmp(tvi.item.pszText, buf)){
				tvi.item.pszText = buf;
				tvi.item.lParam |= addr ? ((addr != ((unsigned)pPPDPTE[i].Addr << 12)) ? ENTRY_ITEM_ADDR_CHANGED : ENTRY_ITEM_CHANGED) : ENTRY_ITEM_NEW;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				SetChildEntryIsChangedFlag(hPagingTree, tvi.item.hItem);
				tvi.item.pszText = text;
			} else{
				tvi.item.mask = TVIF_HANDLE | TVIF_PARAM;
				TreeView_SetItem(hPagingTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
			}
		}
		return 0;
	}
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = pPPTIHs->htCR3;
	for (i = 0; i < 4; i++){
		sprintf(buf, PAE_PDPTE_FORMAT,
			i,
			(unsigned)pPPDPTE[i].Addr << 12,
			pPPDPTE[i].Ign,
			pPPDPTE[i].PCD ? "PCD" : "-",
			pPPDPTE[i].PWT ? "PWT" : "-",
			pPPDPTE[i].P ? "P" : "-"
		);
		tvi.item.pszText = buf;
		pPPTIHs->htPDPTE[i] = TreeView_InsertItem(hPagingTree, &tvi);
	}
	
	return 0;
}

void* ComposePaePdptrItem()
{
	PAGE_DIRECTORY_POINTER_TABLE_REGISTER pdptr;
	PAE_PAGING_TREE_ITEM_HANDLERS* pPPTIHs;
	TVINSERTSTRUCT tvi;
	char buf[0x100];
	void* r;

	pdptr = GetPaePDPTR(hDriverFile);
	r = pdptr.Base << 5;
	pPPTIHs = GetPaePagingTreeItemHandlers(r);
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.hParent = TVI_ROOT;
	sprintf(buf, CR3_FORMAT, r, pdptr.PCD ? "PCD" : "-", pdptr.PWT ? "PWT" : "-", GetCurrentProcessId());
	tvi.item.pszText = buf;
	if (pPPTIHs->htCR3){
		tvi.item.mask |= TVIF_HANDLE | TVIF_PARAM;
		tvi.item.hItem = pPPTIHs->htCR3;
		tvi.item.lParam = 0;
		TreeView_SetItem(hPagingTree, &(tvi.item));
	} else{
		pPPTIHs->htCR3 = TreeView_InsertItem(hPagingTree, &tvi);
	}
	
	return r;
}

// Tree-View Control Reference:
// http://msdn.microsoft.com/en-us/library/cc656646(VS.85).aspx

void ComposePagingTree()
{
	DWORD tid;
	
	PrintLoadingStatus(TRUE);
	bPAE = (((unsigned)ReadCR4(hDriverFile) >> 5) & 1) ? TRUE : FALSE;
	
	if (!bPAE){
		void* pdbr;
		pdbr = ComposePdbrItem();
		ComposePdeItems(pdbr);
		CloseHandle(CreateThread(NULL, 0, ComposePteItems, pdbr, 0, &tid));
	} else{
		void* pdptr;
		pdptr = ComposePaePdptrItem();
		ComposePaePdpteItems(pdptr);
		ComposePaePdeItems(pdptr);
		CloseHandle(CreateThread(NULL, 0, ComposePaePteItems, pdptr, 0, &tid));
	}
	
	CloseHandle(CreateThread(NULL, 0, ComposeCR3Tree, hPagingTree, 0, &tid));
	PrintLoadingStatus(FALSE);

}


int GetTreeViewItemLevel(HWND hwndTV, HTREEITEM hitem)
{

	hitem = TreeView_GetParent(hwndTV, hitem);
	if (hitem == NULL)
		return 0;
		
	return GetTreeViewItemLevel(hwndTV, hitem) + 1;
}

HTREEITEM GetTreeViewItemAncestor(HWND hwndTV, HTREEITEM hitem, int back)
{

	if (back == 0)
		return hitem;

	hitem = TreeView_GetParent(hwndTV, hitem);
	if (hitem == NULL)
		return NULL;	

	return GetTreeViewItemAncestor(hwndTV, hitem, back - 1);
}

void* GetBaseVirtualAddressFromItem(HWND hTree, HTREEITEM hitem)
{
	TVITEM item;
	char buf[0x100], text[0x100], ps[0x10];
	int i;
	unsigned adr = 0, s, t;
	
	item.hItem = hitem;
	item.pszText = text;
	item.cchTextMax = sizeof(text);
	
	while(1){
		item.mask = TVIF_HANDLE | TVIF_TEXT;
		TreeView_GetItem(hTree, &item);
		i = GetTreeViewItemLevel(hTree, item.hItem);
		
		if (!bPAE){
			if (i == 0){
				break;
			} else if (i == 1){
				sscanf(item.pszText, PDE_FORMAT, &s, &t, &t, buf, ps, buf, buf, buf, buf, buf, buf, buf);
				adr += s << 22;
				if (!strcmp(ps, "PS"))
					break;
			} else if (i == 2){
				sscanf(item.pszText, PTE_FORMAT, &s, &t, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
				adr += s << 12;
			}
		} else{
			if (i == 0){
				break;
			} else if (i == 1){
				sscanf(item.pszText, PAE_PDPTE_FORMAT, &s, &t, &t, buf, buf, buf);
				adr += s << 30;
			} else if (i == 2){
				sscanf(item.pszText, PAE_PDE_FORMAT, &s, &t, buf, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
				adr += s << 21;
			} else if (i == 3){
				sscanf(item.pszText, PAE_PTE_FORMAT, &s, &t, buf, &t, buf, buf, buf, buf, buf, buf, buf, buf, buf);
				adr += s << 12;
			}
		}
		
		item.hItem = TreeView_GetParent(hTree, item.hItem);
	}
	
	return (void*)adr;
}

void SetStatusSize(HWND hStatus, int nWidth, int nHeight)
{	
	const int n1 = 12, n2 = 140;
	int nSbSizes[] = {nWidth - (n1 + n2), nWidth - n2, -1};
	
	SendMessage(hStatus, WM_SIZE, nWidth, nHeight);
	SendMessage(hStatus, SB_SETPARTS, 3, nSbSizes);
	
	//PrintStatus(hStatus);
}

void PrintStatus(HWND hStatus, HWND hTree)
{
	char buf[0x100], text[0x100];
	TVITEM item;
	int i;
	unsigned s, adr, t;
	
	buf[0] = 0;	
	if (hTree != INVALID_HANDLE_VALUE){
		item.hItem = TreeView_GetSelection(hTree);
		if (item.hItem){
			item.mask = TVIF_HANDLE | TVIF_TEXT;
			item.pszText = text;
			item.cchTextMax = sizeof(text);
			TreeView_GetItem(hTree, &item);
			i = GetTreeViewItemLevel(hTree, item.hItem);
			if (i > 0){
				adr = GetBaseVirtualAddressFromItem(hTree, item.hItem);
			} else{
				sscanf(item.pszText, CR3_FORMAT, &adr, buf, buf, &t);
			}
			sprintf(buf, "<%s> %08x", i == 0 ? "CR3" : (!bPAE ? (i == 1 ? "PDE" : "PTE") : (i == 1 ? "PDPTE" : i == 2 ? "PDE" : "PTE")), adr);
		}
	}	
	if (!buf[0])
		sprintf(buf, "CR3: %08x", (unsigned)ReadCR3(hDriverFile));
	SendMessage(hStatus, SB_SETTEXT, 0, buf);

	
	t = SendMessage(hStatus, SB_GETTEXT, 2, buf);
	if (!(t & 0xff)){
		s = (unsigned)ReadCR4(hDriverFile);
		__asm__ __volatile__ ("cpuid;": "=a"(t): "a"(0x80000000): "%ebx", "%ecx", "%edx");
		if (t & 0x80000000)
			__asm__ __volatile__ ("cpuid;": "=d"(t): "a"(0x80000001): "%ebx", "%ecx");
		else
			t = 0;
		if (t >> 20)
			t = ReadMSR(hDriverFile, 0xC0000080);	// IA32_EFER
		sprintf(buf, "%s %s %s %s %s", (((unsigned)ReadCR0(hDriverFile) >> 31) & 1 ) ? "PG" : "-", ((s >> 4) & 1) ? "PSE" : "-", ((s >> 5) & 1) ? "PAE" : "-", ((s >> 7) & 1) ? "PGE" : "-", ((t >> 11) & 1) ? "NXE" : "-");
		SendMessage(hStatus, SB_SETTEXT, 2, buf);
	}
}

void PrintLoadingStatus(BOOL loading)
{
	static int count = 0;
	
	count += loading ? 1 : -1;
	if (count < 0)
		count = 0;
	if (count == 1)
		SendMessage(hStatus, SB_SETTEXT, 1, "L");
	else if (count == 0)
		SendMessage(hStatus, SB_SETTEXT, 1, "");
}

static DWORD WINAPI WaitAndInvRectPagingTree(HANDLE h)
{
	PrintLoadingStatus(TRUE);
	WaitForSingleObject(h, 100000);
	PrintLoadingStatus(FALSE);
	InvalidateRect(hPagingTree, NULL, FALSE);
	CloseHandle(h);
}

#define ITEM_LOOKAHEAD_DEAPTH 2

void LoadForwardItems(NMTREEVIEW* pnmtv)
{
	int nForwardExpanded;	// deapth already expanded from selected item
	HTREEITEM hitem, h;
	char buf[0x100], text[0x100];
	TVITEM item;
	int lv = 0;
	unsigned r, t;
	HANDLE ht = NULL;
	
	if (pnmtv->action != TVE_EXPAND)
		return;
	
	item.mask = TVIF_HANDLE | TVIF_TEXT;
	item.pszText = text;
	item.cchTextMax = sizeof(text);

	// Get nForwardExpanded.
	for (nForwardExpanded = 0, hitem = pnmtv->itemNew.hItem; hitem;){
		lv = GetTreeViewItemLevel(pnmtv->hdr.hwndFrom, hitem);
		if (lv == (!bPAE ? 2 : 3))
			break;
		do {
			h = hitem;
			hitem = TreeView_GetChild(pnmtv->hdr.hwndFrom, hitem);
			if (hitem){
				nForwardExpanded++;
				break;
			}
			hitem = h;
			if (lv > 0){
				item.hItem = hitem;
				TreeView_GetItem(pnmtv->hdr.hwndFrom, &item);
				for (r = 0; item.pszText[r] && (r < sizeof(text)); r++);
				r--;
				if (r >= 0 && item.pszText[r] == 'P')
					hitem = NULL;
				else
					hitem = TreeView_GetNextSibling(pnmtv->hdr.hwndFrom, hitem);
			}
		} while(hitem);
	}
	
	if (nForwardExpanded < ITEM_LOOKAHEAD_DEAPTH){	// to load entries is needed
		item.hItem = GetTreeViewItemAncestor(pnmtv->hdr.hwndFrom, pnmtv->itemNew.hItem, GetTreeViewItemLevel(pnmtv->hdr.hwndFrom, pnmtv->itemNew.hItem));
		TreeView_GetItem(pnmtv->hdr.hwndFrom, &item);
		sscanf(item.pszText, "%x s", &r, buf);	// r = cr3
		if (!bPAE){
			if (lv < 2 && 2 <= (lv + ITEM_LOOKAHEAD_DEAPTH)){
				ht = CreateThread(NULL, 0, ComposePteItems, r, 0, &t);
				Sleep(0);
				InvalidateRect(pnmtv->hdr.hwndFrom, NULL, FALSE);
				CloseHandle(CreateThread(NULL, 0, WaitAndInvRectPagingTree, ht, 0, &t));
			}
		} else{
			if (lv < 2 && 2 <= (lv + ITEM_LOOKAHEAD_DEAPTH)){
				ht = CreateThread(NULL, 0, ComposePaePdeItems, r, 0, &t);
				Sleep(0);
				InvalidateRect(pnmtv->hdr.hwndFrom, NULL, FALSE);
				CloseHandle(CreateThread(NULL, 0, WaitAndInvRectPagingTree, ht, 0, &t));
			}
			if (lv < 3 && 3 <= (lv + ITEM_LOOKAHEAD_DEAPTH)){
				ht = CreateThread(NULL, 0, ComposePaePteItems, r, 0, &t);
				Sleep(0);
				InvalidateRect(pnmtv->hdr.hwndFrom, NULL, FALSE);
				CloseHandle(CreateThread(NULL, 0, WaitAndInvRectPagingTree, ht, 0, &t));
			}
		}
	}
}

LRESULT TreeItemExpandingHandler(NMTREEVIEW* pnmtv)
{

	LoadForwardItems(pnmtv);
	return FALSE;
}

void SetChildEntryIsChangedFlag(HWND hwndTV, HTREEITEM hitem)
{
	HTREEITEM hParent;
	TVITEM item;
	
	hParent = TreeView_GetParent(hwndTV, hitem);
	if (!hParent)
		return;
		
	item.mask = TVIF_HANDLE | TVIF_PARAM;
	item.hItem = hParent;
	TreeView_GetItem(hwndTV, &item);
	item.lParam |= CHILD_ENTRY_ITEM_CHANGED;
	TreeView_SetItem(hwndTV, &item);
	SetChildEntryIsChangedFlag(hwndTV, hParent);
}

COLORREF GetTreeViewItemColor(HWND hwndTV, HTREEITEM hItem, NMTVCUSTOMDRAW* pnmcd)
{
	TVITEM tvi;
	
	if (pnmcd)
		if (pnmcd->nmcd.uItemState & CDIS_SELECTED)
			return GetSysColor(COLOR_HIGHLIGHT);
	
	tvi.mask = TVIF_PARAM | TVIF_HANDLE;
	tvi.hItem = hItem;
	TreeView_GetItem(hwndTV, &tvi);
	
	if (tvi.lParam & ENTRY_ITEM_CHANGED)
		return COLOR_CHANGED;
		
	if (tvi.lParam & ENTRY_ITEM_NEW)
		return COLOR_NEW;
		
	if (tvi.lParam & ENTRY_ITEM_ADDR_CHANGED)
		return COLOR_ADDR_CHANGED;
		
	if (tvi.lParam & CHILD_ENTRY_ITEM_CHANGED)
		return COLOR_CHILD_CHANGED;

	return COLOR_DEFAULT;
}

LRESULT TreeItemCustomDrawHandler(NMTVCUSTOMDRAW* pnmcd)
{

	switch(pnmcd->nmcd.dwDrawStage){
		case CDDS_PREPAINT:
			SetWindowLong(GetParent(pnmcd->nmcd.hdr.hwndFrom), DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			pnmcd->clrTextBk = GetTreeViewItemColor(hPagingTree, pnmcd->nmcd.dwItemSpec, pnmcd);
			break;
		default:
			break;
	}
	return NULL;
}

LRESULT ViewerNotifyMessageHandler(NMHDR* pnmhdr)
{

	if (!pnmhdr)
		return;

	if (pnmhdr->idFrom != IDC_PTE_TREE)
		return;
	
	switch(pnmhdr->code){
		case TVN_SELCHANGED:
			PrintStatus(hStatus, hPagingTree);
			break;
		case TVN_ITEMEXPANDING:
			return TreeItemExpandingHandler(pnmhdr);
		case NM_CUSTOMDRAW:
			if (pnmhdr->hwndFrom == hPagingTree)
				return TreeItemCustomDrawHandler(pnmhdr);
			break;
		default:
			break;
	}
	
	return NULL;
}

DWORD WINAPI UpdateTree(void* cr3)
{
	HTREEITEM hitem;
	int i;
	
	if (!bPAE){
		PAGING_TREE_ITEM_HANDLERS* p;
		if (!cr3){
			PrintLoadingStatus(TRUE);
			ComposePdbrItem();
			ComposeCR3Tree(hPagingTree);
			for (p = pFirstPtiHandlers; p; p = p->next){
				UpdateTree(p->pdbr);
			}
			PrintLoadingStatus(FALSE);
		} else{
			p = GetPagingTreeItemHandlers(cr3);
			if (!(p->htPDE[0]))
				return 0;
			ComposePdeItems(p->pdbr);
			for (i = 0; i < 1024; i++){
				if (TreeView_GetChild(hPagingTree, p->htPDE[i])){
					ComposePteItems(p->pdbr);
					break;
				}
			}
		}
	} else{
		PAE_PAGING_TREE_ITEM_HANDLERS* p;
		if (!cr3){
			PrintLoadingStatus(TRUE);
			ComposePaePdptrItem();
			ComposeCR3Tree(hPagingTree);
			for (p = pFirstPtiHandlers; p; p = p->next){
				UpdateTree(p->pdptr);
			}
			PrintLoadingStatus(FALSE);
		} else{
			p = GetPagingTreeItemHandlers(cr3);
			if (!(p->htPDPTE[0]))
				return 0;
			ComposePaePdpteItems(p->pdptr);
			if (!(p->htPDE[0][0]))
				return 0;
			ComposePaePdeItems(p->pdptr);
			for (i = 0; i < 512; i++){
				if (TreeView_GetChild(hPagingTree, p->htPDE[0][i])){
					ComposePaePteItems(p->pdptr);
					break;
				}
			}
		}
	}
	InvalidateRect(hPagingTree, NULL, FALSE);
	return 0;
}


PAGING_TREE_ITEM_HANDLERS* GetPagingTreeItemHandlers(void* pdbr)
{
	return (PAGING_TREE_ITEM_HANDLERS*)GetListItem(pdbr, &pFirstPtiHandlers, sizeof(PAGING_TREE_ITEM_HANDLERS));
}

void DelPagingTreeItemHandlers(void* pdbr)
{
	DeleteListItem(pdbr, pFirstPtiHandlers, sizeof(PAGING_TREE_ITEM_HANDLERS));
}

PAE_PAGING_TREE_ITEM_HANDLERS* GetPaePagingTreeItemHandlers(void* pdptr)
{
	return (PAE_PAGING_TREE_ITEM_HANDLERS*)GetListItem(pdptr, &pFirstPtiHandlers, sizeof(PAE_PAGING_TREE_ITEM_HANDLERS));
}

void DelPaePagingTreeItemHandlers(void* pdptr)
{
	DeleteListItem(pdptr, pFirstPtiHandlers, sizeof(PAE_PAGING_TREE_ITEM_HANDLERS));
}


void ShowContextMenu(HWND hwnd, LPARAM lp)
{
	TV_HITTESTINFO hi;
	HTREEITEM hItem;
	
	hi.pt.x = LOWORD(lp);
	hi.pt.y = HIWORD(lp);
	ScreenToClient(hPagingTree, &hi.pt);
	hItem = TreeView_HitTest(hPagingTree, &hi);
	if (hItem){
		 TreeView_SelectItem(hPagingTree, hItem);
		 TrackPopupMenu(hContextMenu, TPM_LEFTALIGN | TPM_TOPALIGN, LOWORD(lp), HIWORD(lp), 0, hwnd, NULL);
	}
}

void TestCache(void* addr)
{
	unsigned results[3] = {0};
	char buf[0x100];
	CheckAccessLatency(hDriverFile, addr, results, sizeof(results));
	sprintf(buf, "Address: %08xh\nData Latency: %d\nInst Latency: %d", addr, results[0], results[1]);
	MessageBox(hPagingTree, buf, "Access Test", MB_OK);
}
