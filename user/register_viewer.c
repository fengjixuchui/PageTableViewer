#include <windows.h>
#include "specialreg.h"
#include "kernel_interface.h"

extern HANDLE hDriverFile;
HWND hRegisterViewerEdit;
HWND hDemandStatic;
HWND hMsrNumEdit;
HWND hMsrValEdit;
HWND hMsrReadButton;
HWND hMsrWriteButton;

void AddNewline(char** p)
{
	for (; **p; (*p)++);
	**p = '\r'; (*p)++;
	**p = '\n'; (*p)++;
}

#define ComposeMsrList(name, p) {	\
	LONGLONG r = ReadMSR(hDriverFile, name);	\
	sprintf((p), #name "(%xh): %04x %04x %04x %04xh\r\n", name, (unsigned short)(r >> 48), (unsigned short)(r >> 32), (unsigned short)(r >> 16), (unsigned short)r);	\
	for(; *(p); (p)++);	\
}

DWORD WINAPI ComposeMsrLists(HWND hEdit)
{
	char buf[0x10000];
	char* p = buf;
	
	sprintf(p, "CR0: %08xh\r\n", (unsigned)ReadCR0(hDriverFile)); for(; *p; p++);
	sprintf(p, "CR4: %08xh\r\n", (unsigned)ReadCR4(hDriverFile)); for(; *p; p++);
	sprintf(p, "---\r\n"); for(; *p; p++);
	
	ComposeMsrList(MSR_P5_MC_ADDR, p);
	ComposeMsrList(MSR_P5_MC_TYPE, p);
	ComposeMsrList(MSR_TSC, p);
	//ComposeMsrList(MSR_P5_CESR, p);
	//ComposeMsrList(MSR_P5_CTR0, p);
	//ComposeMsrList(MSR_P5_CTR1, p);
	ComposeMsrList(MSR_IA32_PLATFORM_ID, p);
	ComposeMsrList(MSR_APICBASE, p);
	ComposeMsrList(MSR_EBL_CR_POWERON, p);
	//ComposeMsrList(MSR_TEST_CTL, p);
	//ComposeMsrList(MSR_BIOS_UPDT_TRIG, p);
	//ComposeMsrList(MSR_BBL_CR_D0, p);
	//ComposeMsrList(MSR_BBL_CR_D1, p);
	//ComposeMsrList(MSR_BBL_CR_D2, p);
	ComposeMsrList(MSR_BIOS_SIGN, p);
	ComposeMsrList(MSR_PERFCTR0, p);
	ComposeMsrList(MSR_PERFCTR1, p);
	//ComposeMsrList(MSR_IA32_EXT_CONFIG, p);	/* Undocumented. Core Solo/Duo only */
	ComposeMsrList(MSR_MTRRcap, p);
	//ComposeMsrList(MSR_BBL_CR_ADDR, p);
	//ComposeMsrList(MSR_BBL_CR_DECC, p);
	//ComposeMsrList(MSR_BBL_CR_CTL, p);
	//ComposeMsrList(MSR_BBL_CR_TRIG, p);
	//ComposeMsrList(MSR_BBL_CR_BUSY, p);
	//ComposeMsrList(MSR_BBL_CR_CTL3, p);
	ComposeMsrList(MSR_SYSENTER_CS_MSR, p);
	ComposeMsrList(MSR_SYSENTER_ESP_MSR, p);
	ComposeMsrList(MSR_SYSENTER_EIP_MSR, p);
	ComposeMsrList(MSR_MCG_CAP, p);
	ComposeMsrList(MSR_MCG_STATUS, p);
	//ComposeMsrList(MSR_MCG_CTL, p);
	//ComposeMsrList(MSR_EVNTSEL0, p);
	//ComposeMsrList(MSR_EVNTSEL1, p);
	ComposeMsrList(MSR_THERM_CONTROL, p);
	ComposeMsrList(MSR_THERM_INTERRUPT, p);
	ComposeMsrList(MSR_THERM_STATUS, p);
	ComposeMsrList(MSR_IA32_MISC_ENABLE, p);
	ComposeMsrList(MSR_DEBUGCTLMSR, p);
	ComposeMsrList(MSR_LASTBRANCHFROMIP, p);
	ComposeMsrList(MSR_LASTBRANCHTOIP, p);
	ComposeMsrList(MSR_LASTINTFROMIP, p);
	ComposeMsrList(MSR_LASTINTTOIP, p);
	//ComposeMsrList(MSR_ROB_CR_BKUPTMPDR6, p);
	ComposeMsrList(MSR_MTRRVarBase, p);
	ComposeMsrList(MSR_MTRRVarMask, p);
	ComposeMsrList(MSR_MTRRVarBase+2, p);
	ComposeMsrList(MSR_MTRRVarMask+2, p);
	ComposeMsrList(MSR_MTRRVarBase+4, p);
	ComposeMsrList(MSR_MTRRVarMask+4, p);
	ComposeMsrList(MSR_MTRRVarBase+6, p);
	ComposeMsrList(MSR_MTRRVarMask+6, p);
	ComposeMsrList(MSR_MTRRVarBase+8, p);
	ComposeMsrList(MSR_MTRRVarMask+8, p);
	ComposeMsrList(MSR_MTRRVarBase+10, p);
	ComposeMsrList(MSR_MTRRVarMask+10, p);
	ComposeMsrList(MSR_MTRRVarBase+12, p);
	ComposeMsrList(MSR_MTRRVarMask+12, p);
	ComposeMsrList(MSR_MTRRVarBase+14, p);
	ComposeMsrList(MSR_MTRRVarMask+14, p);
	ComposeMsrList(MSR_MTRR64kBase, p);
	ComposeMsrList(MSR_MTRR16kBase, p);
	ComposeMsrList(MSR_MTRR16kBase+1, p);
	ComposeMsrList(MSR_MTRR4kBase, p);
	ComposeMsrList(MSR_MTRR4kBase+1, p);
	ComposeMsrList(MSR_MTRR4kBase+2, p);
	ComposeMsrList(MSR_MTRR4kBase+3, p);
	ComposeMsrList(MSR_MTRR4kBase+4, p);
	ComposeMsrList(MSR_MTRR4kBase+5, p);
	ComposeMsrList(MSR_MTRR4kBase+6, p);
	ComposeMsrList(MSR_MTRR4kBase+7, p);
	ComposeMsrList(MSR_PAT, p);
	ComposeMsrList(MSR_MTRRdefType, p);
	//ComposeMsrList(MSR_MC0_CTL, p);
	//ComposeMsrList(MSR_MC0_STATUS, p);
	//ComposeMsrList(MSR_MC0_ADDR, p);
	//ComposeMsrList(MSR_MC0_MISC, p);
	//ComposeMsrList(MSR_MC1_CTL, p);
	//ComposeMsrList(MSR_MC1_STATUS, p);
	//ComposeMsrList(MSR_MC1_ADDR, p);
	//ComposeMsrList(MSR_MC1_MISC, p);
	//ComposeMsrList(MSR_MC2_CTL, p);
	//ComposeMsrList(MSR_MC2_STATUS, p);
	//ComposeMsrList(MSR_MC2_ADDR, p);
	//ComposeMsrList(MSR_MC2_MISC, p);
	//ComposeMsrList(MSR_MC3_CTL, p);
	//ComposeMsrList(MSR_MC3_STATUS, p);
	//ComposeMsrList(MSR_MC3_ADDR, p);
	//ComposeMsrList(MSR_MC3_MISC, p);
	//ComposeMsrList(MSR_MC4_CTL, p);
	//ComposeMsrList(MSR_MC4_STATUS, p);
	//ComposeMsrList(MSR_MC4_ADDR, p);
	//ComposeMsrList(MSR_MC4_MISC, p);
	*p = '\0';
	
	SetWindowText(hEdit, buf);

	return 0;
}

LRESULT CALLBACK RegisterViewerProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	RECT rMain, rStatic;
	DWORD tid;
	char buf[0x20];
	unsigned r;
	LONGLONG v;
	int i;
	
	switch(msg){
		case WM_CREATE:
			hRegisterViewerEdit = CreateWindow(
				TEXT("EDIT"), TEXT(""),
				WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE,
				0, 40, 400, 460,
				hwnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			CloseHandle(CreateThread(NULL, 0, ComposeMsrLists, hRegisterViewerEdit, 0, &tid));
			
			hDemandStatic = CreateWindow(
				TEXT("STATIC"), TEXT(""),
				WS_CHILD | WS_VISIBLE,
				0, 0, 400, 40,
				hwnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			CreateWindow(
				TEXT("Static"), TEXT("MSR:"),
				WS_CHILD | WS_VISIBLE,
				0, 10, 40, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			hMsrNumEdit = CreateWindow(
				TEXT("EDIT"), TEXT(""),
				WS_CHILD | WS_VISIBLE,
				40, 10, 40, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			CreateWindow(
				TEXT("Static"), TEXT("h"),
				WS_CHILD | WS_VISIBLE,
				80, 10, 10, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			CreateWindow(
				TEXT("Static"), TEXT("Val:"),
				WS_CHILD | WS_VISIBLE,
				100, 10, 40, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			hMsrValEdit = CreateWindow(
				TEXT("EDIT"), TEXT(""),
				WS_CHILD | WS_VISIBLE,
				130, 10, 120, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			CreateWindow(
				TEXT("Static"), TEXT("h"),
				WS_CHILD | WS_VISIBLE,
				250, 10, 10, 20,
				hDemandStatic,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			hMsrReadButton = CreateWindow(
				TEXT("BUTTON"), TEXT("Read"),
				WS_CHILD | WS_VISIBLE,
				270, 10, 50, 20,
				hwnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			hMsrWriteButton = CreateWindow(
				TEXT("BUTTON"), TEXT("Write"),
				WS_CHILD | WS_VISIBLE,
				330, 10, 50, 20,
				hwnd,
				NULL,
				(HINSTANCE)GetModuleHandle(NULL),
				NULL
			);
			
			break;
		case WM_SIZE:
			GetClientRect(hwnd, &rMain);
			GetClientRect(hDemandStatic, &rStatic);
			SetWindowPos(hDemandStatic, NULL, 0, 0, lp & 0xffff, rStatic.bottom, 0);
			SetWindowPos(hRegisterViewerEdit, NULL, 0, rStatic.bottom, rMain.right, rMain.bottom - rStatic.bottom, 0);
			break;
		case WM_COMMAND:
			if (lp == hMsrReadButton){
				GetWindowText(hMsrNumEdit, buf, sizeof(buf) / sizeof(char));
				sscanf(buf, "%x", &r);
				v = ReadMSR(hDriverFile, r);
				(unsigned)(v >> 32) ? sprintf(buf, "%x%08x", (unsigned)(v >> 32), (unsigned)v) : sprintf(buf, "%x", (unsigned)v);
				SetWindowText(hMsrValEdit, buf);
			} else if (lp == hMsrWriteButton){
				GetWindowText(hMsrValEdit, buf, sizeof(buf) / sizeof(char));
				for (v = 0, i = 0; buf[i]; i++){
					v *= 0x10;
					v += buf[i] - ( (buf[i] >= '0' && buf[i] <= '9') ? '0' : (((buf[i] >= 'a' && buf[i] <= 'f') ? 'a' : 'A') - 0xa) );	
				}
				GetWindowText(hMsrNumEdit, buf, sizeof(buf) / sizeof(char));
				r = strtoul(buf, NULL, 16);
				WriteMSR(hDriverFile, r, v);
			}
			break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

void CreateRegisterViewer(HWND hParent)
{
	WNDCLASS winc;

	winc.style			= CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc	= RegisterViewerProc;
	winc.cbClsExtra		= winc.cbWndExtra = 0;
	winc.hInstance		= (HINSTANCE)GetModuleHandle(NULL);
	winc.hIcon			= LoadIcon(NULL , IDI_APPLICATION);
	winc.hCursor		= LoadCursor(NULL , IDC_ARROW);
	winc.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName	= NULL;
	winc.lpszClassName	= TEXT("REGISTER_VIEWER");
	
	RegisterClass(&winc);

	CreateWindow(
		TEXT("REGISTER_VIEWER"), TEXT("Register Viewer"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, 400, 500,
		hParent,
		NULL,
		(HINSTANCE)GetModuleHandle(NULL),
		NULL
	);
}

