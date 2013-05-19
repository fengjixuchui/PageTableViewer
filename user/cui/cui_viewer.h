#ifndef __CUI_VIEWER_H__
#define __CUI_VIEWER_H__

#define PTE_FORMAT "%04d: %08x %1d %s %s %s %s %s %s %s %s %s"
#define PDE_FORMAT PTE_FORMAT
#define CR3_FORMAT "%08x %s %s <PID:%5d>"
#define PAE_PDPTE_FORMAT "%01d: %09x %1d %s %s %s"
#define PAE_PTE_FORMAT "%03d: %09x %s %1d %s %s %s %s %s %s %s %s %s"
#define PAE_PDE_FORMAT PAE_PTE_FORMAT

void PrintPagingTree(int pid);	// -1 (not implemented): print all; -2: print myself;

#endif /* __CUI_VIEWER_H__ */