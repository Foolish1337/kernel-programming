#include <ntifs.h>
#include <ntddk.h>

#include "PriorityBoosterCommon.h"

// Driver Unload
void PriorityBoosterUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING uSymLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
	// delete symbolic link and device object
	IoDeleteSymbolicLink(&uSymLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	KdPrint(("PriorityBooster unloaded\n"));
}

// Accepts pointer to device object and pointer to I/O Request Packet (IRP)
_Use_decl_annotations_
NTSTATUS PriorityBoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) 
{
	UNREFERENCED_PARAMETER(DeviceObject);
	
	Irp->IoStatus.Status = STATUS_SUCCESS; // indicating the status this request would complete with
	Irp->IoStatus.Information = 0;         // polymorphic member (different things in different request, Create and Close zero is fine)
	
	// complete IRP. Propagates the IRP back to creator
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS PriorityBoosterDeviceControl(PDEVICE_OBJECT, PIRP Irp)
{	
	// get our IO_STACK_LOCATION
	auto stack = IoGetCurrentIrpStackLocation(Irp); // IO_STACK_LOCATION*
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_PRIORITY_BOOSTER_SET_PRIORITY: 
	{
		// check if buffer recieved is large enough
		if (stack->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		// after that check we can automatically assume the buffer is large enough so we cast it to ThreadData
		auto data = (ThreadData*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

		// If pointer is null we should abort (obvious reasons why)
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		// check if priority is in correct range from 1 -> 31
		if (data->Priority < 1 || data->Priority > 31) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		PETHREAD thread;
		status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread); // change threadID to actual thread obj
		if (!NT_SUCCESS(status))
			break;

		KeSetPriorityThread((PKTHREAD)thread, data->Priority);
		ObDereferenceObject(thread);
		KdPrint(("Thread priority changed for %d -> %d succeeded\n", data->ThreadId, data->Priority));;
		break;
	}
	default:
		KdPrint(("Invalid device request...\n"));
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

// Driver Entry
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = PriorityBoosterUnload;

	// Setting up dispatch routines (all drivers must support IRP_MJ_CREATE + IRP_MJ_CLOSE otherwise no way to open handle)
	DriverObject->MajorFunction[IRP_MJ_CREATE] = PriorityBoosterCreateClose; 
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = PriorityBoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PriorityBoosterDeviceControl;

	UNICODE_STRING uDeviceName = RTL_CONSTANT_STRING(L"\\Device\\PriorityBooster");
	PDEVICE_OBJECT deviceObject;

	// Get pointer to our device object
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &uDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	// Make device object accessible to user mode callers by providing symbolic link
	UNICODE_STRING uSymLink = RTL_CONSTANT_STRING(L"\\??\\PriorityBooster");
	status = IoCreateSymbolicLink(&uSymLink, &uDeviceName);
	KdPrint(("Created symbolic link\n"));

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(deviceObject);
		return status;
	}

	return STATUS_SUCCESS;
}