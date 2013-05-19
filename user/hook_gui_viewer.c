#include <windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include <stdio.h>
#include "hook_caller.h"
#include "hook_gui_viewer.h"
#include "gui_viewer.h"

extern BOOL bPAE;

DWORD ComposeCR3Tree(HWND hTree)
{
	TVINSERTSTRUCT tvi;
	DWORD pid = GetCurrentProcessId();
	void* cr3;
	char buf[0x80];
	
	HANDLE hSnp;
	PROCESSENTRY32 pe;
	BOOL b;
	
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT;
	tvi.item.lParam = 0;
	
	tvi.hParent = TVI_ROOT;
	
	hSnp = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnp == INVALID_HANDLE_VALUE)
		return 0;
	pe.dwSize = sizeof(PROCESSENTRY32);
	
	PrintLoadingStatus(TRUE);
	for(b = Process32First(hSnp, &pe); b; b = Process32Next(hSnp, &pe)){
		if (pe.th32ProcessID == pid)
			continue;
		cr3 = GetCR3ofPid(pe.th32ProcessID);
		if (!cr3)
			continue;
		tvi.item.pszText = buf;
		if (!bPAE){
			PAGING_TREE_ITEM_HANDLERS* pPTIHs;
			pPTIHs = GetPagingTreeItemHandlers((unsigned)cr3 & 0xfffff000);
			sprintf(buf, "%08x <PID:%5d> [%s]", cr3, pe.th32ProcessID, pe.szExeFile);
			if (pPTIHs->htCR3){
				tvi.item.mask |= TVIF_HANDLE | TVIF_PARAM;
				tvi.item.hItem = pPTIHs->htCR3;
				TreeView_SetItem(hTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT;
			} else{
				pPTIHs->htCR3 = TreeView_InsertItem(hTree, &tvi);
			}
			ComposePdeItems((unsigned)cr3 & 0xfffff000);
		} else{
			PAE_PAGING_TREE_ITEM_HANDLERS* pPPTIHs;
			pPPTIHs = GetPaePagingTreeItemHandlers((unsigned)cr3 & 0xffffffe0);
			sprintf(buf, "%08x <PID:%5d> [%s]", cr3, pe.th32ProcessID, pe.szExeFile);
			if (pPPTIHs->htCR3){
				tvi.item.mask |= TVIF_HANDLE | TVIF_PARAM;
				tvi.item.hItem = pPPTIHs->htCR3;
				TreeView_SetItem(hTree, &(tvi.item));
				tvi.item.mask = TVIF_TEXT;
			} else{
				pPPTIHs->htCR3 = TreeView_InsertItem(hTree, &tvi);
			}
			ComposePaePdpteItems((unsigned)cr3 & 0xffffffe0);
		}
	}
	PrintLoadingStatus(FALSE);
	
	CloseHandle(hSnp);
	InvalidateRect(hTree, NULL, FALSE);
	
	return 0;
}

void WindupHookViewer()
{

	FreeRemoteHandler(-1);
}

