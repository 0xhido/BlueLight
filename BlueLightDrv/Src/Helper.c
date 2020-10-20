#include "Helper.h"
#include "Undocumented.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

#define HELPER_TAG 'hLB' // BLh - BlueLight Helper

////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////

NTSTATUS QuerySystemInformation(
	SYSTEM_INFORMATION_CLASS InfoClass, 
	PVOID* InfoBuffer, 
	PSIZE_T InfoSize
) {
	PVOID info = NULL;
	NTSTATUS status;
	ULONG size = 0, written = 0;

	// Query required size
	status = ZwQuerySystemInformation(InfoClass, info, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return status;

	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		size += written; // We should allocate little bit more space

		if (info)
			ExFreePoolWithTag(info, HELPER_TAG);

		info = ExAllocatePoolWithTag(NonPagedPool, size, HELPER_TAG);
		if (!info)
			break;

		status = ZwQuerySystemInformation(InfoClass, info, size, &written);
	}

	if (!info)
		return STATUS_ACCESS_DENIED;

	if (!NT_SUCCESS(status))
	{
		ExFreePoolWithTag(info, HELPER_TAG);
		return status;
	}

	*InfoBuffer = info;
	*InfoSize = size;

	return status;
}

NTSTATUS QueryProcessInformation(PROCESSINFOCLASS Class, HANDLE Process, PVOID* InfoBuffer, PSIZE_T InfoSize)
{
	PVOID info = NULL;
	NTSTATUS status;
	ULONG size = 0, written = 0;

	// Query required size
	status = ZwQueryInformationProcess(Process, Class, 0, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return status;

	while (status == STATUS_INFO_LENGTH_MISMATCH)
	{
		size += written; // We should allocate little bit more space

		if (info)
			ExFreePoolWithTag(info, HELPER_TAG);

		info = ExAllocatePoolWithTag(NonPagedPool, size, HELPER_TAG);
		if (!info)
			break;

		status = ZwQueryInformationProcess(Process, Class, info, size, &written);
	}

	if (!info)
		return STATUS_ACCESS_DENIED;

	if (!NT_SUCCESS(status))
	{
		ExFreePoolWithTag(info, HELPER_TAG);
		return status;
	}

	*InfoBuffer = info;
	*InfoSize = size;

	return status;
}


VOID FreeInformation(PVOID Buffer) {
	ExFreePoolWithTag(Buffer, HELPER_TAG);
}
