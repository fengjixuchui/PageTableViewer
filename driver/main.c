#define __KERNEL__

#include <ntddk.h>
#include "ptviewer.h"
#include "func.h"

#define USDRIVERNAME	L"\\Device\\PAGE_TABLE_VIEWER"
#define	USDOSDRIVERNAME	L"\\DosDevices\\PAGE_TABLE_VIEWER"

#define __file__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1) : __FILE__)
#define __printd(format, ...) DbgPrint("[%s %d] "format , __file__, __LINE__, __VA_ARGS__ +0)

NTSTATUS VT_MEASUREMENT_UnSupportedFunction(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp);
NTSTATUS VT_MEASUREMENT_DeviceOpen(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp);
NTSTATUS VT_MEASUREMENT_DeviceClose(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp);
NTSTATUS VT_MEASUREMENT_DeviceRead(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp);
NTSTATUS VT_MEASUREMENT_IoControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);

VOID VT_MEASUREMENT_DriverUnload(IN PDRIVER_OBJECT pDriverObject);


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS			Status = STATUS_SUCCESS;
	PDEVICE_OBJECT		pDeviceObject = NULL;
	UNICODE_STRING usDriverName, usDosDeviceName;
	int i;
	
	__printd("DriverEntry\n");

	RtlInitUnicodeString(&usDriverName, USDRIVERNAME);
    RtlInitUnicodeString(&usDosDeviceName, USDOSDRIVERNAME); 
	IoCreateSymbolicLink(&usDosDeviceName, &usDriverName);

	for(i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
         pDriverObject->MajorFunction[i] = VT_MEASUREMENT_UnSupportedFunction;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = VT_MEASUREMENT_DeviceOpen;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = VT_MEASUREMENT_DeviceClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = VT_MEASUREMENT_DeviceRead;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VT_MEASUREMENT_IoControl;
	pDriverObject->DriverUnload = VT_MEASUREMENT_DriverUnload;
	
	Status = IoCreateDevice(pDriverObject,0,&usDriverName,FILE_DEVICE_UNKNOWN,0,FALSE,&pDeviceObject);
	if (!NT_SUCCESS(Status)){
		__printd("DriverEntry IoCreateDevice() failed.\n");
		return STATUS_UNSUCCESSFUL;
	}
	
	return STATUS_SUCCESS;
}

NTSTATUS VT_MEASUREMENT_UnSupportedFunction(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{

	__printd("UnSupportedFunction(%x)\n", IoGetCurrentIrpStackLocation(pIrp)->MajorFunction);

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS VT_MEASUREMENT_DeviceOpen(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{

	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS VT_MEASUREMENT_DeviceClose(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{

	__printd("DeviceClose(%x)\n", IoGetCurrentIrpStackLocation(pIrp)->MajorFunction);
	
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS VT_MEASUREMENT_DeviceRead(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	PCHAR ioBuffer;
	ULONG outputBufferLength = 0;
	NTSTATUS ntStatus = STATUS_SUCCESS;

	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(pIrp);
	
	ioBuffer = pIrp->AssociatedIrp.SystemBuffer;
	if (!ioBuffer)
		ioBuffer = pIrp->UserBuffer;
	outputBufferLength = irpStack->Parameters.Read.Length;
	if (!ioBuffer){
		__printd("VT_MEASUREMENT_DeviceRead(%x) : NULL, %x, %x\n", IoGetCurrentIrpStackLocation(pIrp)->MajorFunction, ioBuffer, pIrp->Flags);
		goto READ_FUNC_END;
	}
	
	__printd("DeviceRead(%x) : %x, %d\n", IoGetCurrentIrpStackLocation(pIrp)->MajorFunction, ioBuffer, outputBufferLength);
	
	if (0){
		if (outputBufferLength >= sizeof(int)){
			static int i = 100;
			*((__int64*)ioBuffer) = i++;
			outputBufferLength = sizeof(int);
		}
	}
	
	ntStatus = STATUS_SUCCESS;
	
READ_FUNC_END:
	pIrp->IoStatus.Information = outputBufferLength;
	pIrp->IoStatus.Status = ntStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return ntStatus;
}

NTSTATUS VT_MEASUREMENT_IoControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp)
{
	PCHAR inputBuffer;
	PCHAR outputBuffer;
	ULONG inputBufferLength = 0;
	ULONG outputBufferLength = 0;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG controlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
	
	PHYSICAL_ADDRESS phys;
	
	inputBuffer =
		( (controlCode & 0x03) == METHOD_BUFFERED || (controlCode & 0x03) == METHOD_OUT_DIRECT ) ?  pIrp->AssociatedIrp.SystemBuffer :
		(controlCode & 0x03) == METHOD_IN_DIRECT ?  pIrp->MdlAddress :
		(controlCode & 0x03) == METHOD_NEITHER ?  irpStack->Parameters.DeviceIoControl.Type3InputBuffer :
		NULL;
	outputBuffer =
		(controlCode & 0x03) == METHOD_BUFFERED ?  pIrp->AssociatedIrp.SystemBuffer :
		( (controlCode & 0x03) == METHOD_IN_DIRECT || (controlCode & 0x03) == METHOD_OUT_DIRECT ) ?  pIrp->MdlAddress :
		(controlCode & 0x03) == METHOD_NEITHER ?  pIrp->UserBuffer :
		NULL;
		
	inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	if (!inputBuffer && !outputBuffer && (inputBufferLength || outputBufferLength)){
		__printd("VT_MEASUREMENT_IoControl(%x) : NULL, o_buf: %x, %x, i_len: %d, o_len: %d, CC: %d\n", IoGetCurrentIrpStackLocation(pIrp)->MajorFunction, outputBuffer, pIrp->Flags, inputBufferLength, outputBufferLength, irpStack->Parameters.DeviceIoControl.IoControlCode);
		goto IOCTL_FUNC_END;
	}
	
	switch (controlCode){
		case IOCTL_READ_CR3:
			*((unsigned __int64*)outputBuffer) = __readcr3();
			outputBufferLength = sizeof(unsigned __int64);
			break;
		case IOCTL_GET_PHYSICAL_ADDRESS:
			*((PHYSICAL_ADDRESS*)outputBuffer) = MmGetPhysicalAddress(*((void**)inputBuffer));
			outputBufferLength = sizeof(PHYSICAL_ADDRESS);
			break;
		case IOCTL_COPY_PHYSICAL_MEMORY:
			phys.LowPart = *((unsigned*)inputBuffer);
			phys.HighPart = ((inputBufferLength > 4) ? ((unsigned*)(inputBuffer))[1] : 0);
			CopyPhysMemory(outputBuffer, phys, outputBufferLength);
			break;
		case IOCTL_COPY_MEMORY:
			RtlMoveMemory(outputBuffer, *(void**)inputBuffer, outputBufferLength);
			break;
		
		case IOCTL_READ_CR0:
			*((unsigned __int64*)outputBuffer) = __readcr0();
			outputBufferLength = sizeof(unsigned __int64);
			break;
		case IOCTL_READ_CR2:
			*((unsigned __int64*)outputBuffer) = __readcr2();
			outputBufferLength = sizeof(unsigned __int64);
			break;
		case IOCTL_READ_CR4:
			*((unsigned __int64*)outputBuffer) = __readcr4();
			outputBufferLength = sizeof(unsigned __int64);
			break;
		case IOCTL_WRITE_CR0:
			__writecr0(*((unsigned __int64*)outputBuffer));
			outputBufferLength = 0;
			break;
		case IOCTL_WRITE_CR3:
			__writecr3(*((unsigned __int64*)outputBuffer));
			outputBufferLength = 0;
			break;
		case IOCTL_WRITE_CR4:
			__writecr4(*((unsigned __int64*)outputBuffer));
			outputBufferLength = 0;
			break;
			
		case IOCTL_READ_MSR:
			*((unsigned __int64*)outputBuffer) = __readmsr(*(unsigned*)inputBuffer);
			outputBufferLength = sizeof(unsigned __int64);
			break;
		case IOCTL_WRITE_MSR:
			__writemsr(*(unsigned*)inputBuffer, inputBufferLength == 8 ? (unsigned __int64)(((unsigned *)inputBuffer)[1] & 0xffffffff) : *(unsigned __int64 *)(((unsigned *)inputBuffer) + 1));
			outputBufferLength = 0;
			break;
			
		case IOCTL_DO_WBINVD:
			__wbinvd();
			outputBufferLength = 0;
			break;
		case IOCTL_DO_INVLPG:
			__invlpg(*(void**)inputBuffer);
			outputBufferLength = 0;
			break;	
			
		case IOCTL_CHECK_ACCESS_LATENCY:
			CheckAccessLatency(*(void**)inputBuffer, outputBuffer, outputBufferLength);
			outputBufferLength = (outputBufferLength > 3 * sizeof(unsigned) ? 3 * sizeof(unsigned) : outputBufferLength);
			break;
			
	/*
		case IOCTL_SET_TARGET:
			__printd("VT_MEASUREMENT_IoControl : IOCTL_SET_TARGET %08x", read_cr3());
			setTargetCr3(read_cr3(), MEASURE_TLB);
			break;
		case IOCTL_GET_INSTRUCTION_TLB_INFO:
		case IOCTL_GET_DATA_TLB_INFO:
			if (!outputBuffer) break;
			if (inputBuffer && inputBufferLength >= sizeof(KAFFINITY)){
				affinity = *((KAFFINITY*)inputBuffer);
				KeSetSystemAffinityThread(affinity);
			}
			if (outputBufferLength >= sizeof(TlbInfo)){
				TlbInfo tlbInfo;
				if (controlCode == IOCTL_GET_INSTRUCTION_TLB_INFO){
					getTlbInfo(&tlbInfo, NULL);
				} else if (controlCode == IOCTL_GET_DATA_TLB_INFO){
					getTlbInfo(NULL, &tlbInfo);
				}
				*((TlbInfo*)(outputBuffer)) = tlbInfo;
				outputBufferLength = sizeof(TlbInfo);
			}
			break;
	*/
		default:
			break;
	}

IOCTL_FUNC_END:
	pIrp->IoStatus.Information = outputBufferLength;
	pIrp->IoStatus.Status = ntStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return ntStatus;
}

VOID VT_MEASUREMENT_DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING usDosDeviceName;
	
	__printd("VT_MEASUREMENT_DriverUnload\n");

	RtlInitUnicodeString(&usDosDeviceName, USDOSDRIVERNAME);
	IoDeleteSymbolicLink(&usDosDeviceName);
	IoDeleteDevice(pDriverObject->DeviceObject);

}
