#include <ntddk.h>

#include "Monitor.h"

#include "Data/ProcessTable.h"

#include "Callbacks/CbProcess.h"
#include "Callbacks/CbThread.h"
#include "Callbacks/CbImage.h"

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

FAST_MUTEX g_ProcessTableLock;

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeMonitor(
	PDRIVER_OBJECT DriverObject
) {
	NTSTATUS status;
	
	UNREFERENCED_PARAMETER(DriverObject);

	ExInitializeFastMutex(&g_ProcessTableLock);

	status = InitializeProcessTable();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = InitializeProcessCallbacks();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = InitializeThreadCallbacks();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = InitializeImageCallbacks();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	return STATUS_SUCCESS;
}


VOID DestroyMonitor() {
	DestroyProcessCallbacks();
	DestroyThreadCallbacks();
	DestroyImageCallbacks();
	DestroyProcessTable();
}
