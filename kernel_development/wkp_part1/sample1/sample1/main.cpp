#include <ntddk.h>

void* operator new(
	_In_ size_t NumberOfBytes,
	_In_ POOL_TYPE PoolType,
	_In_ ULONG Tag = 0) 
{
	auto tmp = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);

	if (tmp == nullptr)
		return nullptr;

	return tmp;
}

void SampleUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("Sample driver unload called...\n"));
}

/*
Considered the "main" function. Functions is called by a system thread at IRQL_PASSIVE_LEVEL(0)
*/
extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	DriverObject->DriverUnload = SampleUnload;
	RTL_OSVERSIONINFOW pointerOut;

	pointerOut.dwOSVersionInfoSize = sizeof(pointerOut);
	NTSTATUS status = RtlGetVersion(&pointerOut);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("RtlGetVersion(); failed: %d\n", status));
		return -1;
	}

	KdPrint(("Windows version information: Major: %d, Minor: %d, Build: %d\n", 
		pointerOut.dwMajorVersion, pointerOut.dwMinorVersion, pointerOut.dwBuildNumber));
	return STATUS_SUCCESS;
}