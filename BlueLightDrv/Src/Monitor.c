#include "Monitor.h"

#include "Callbacks/CbProcess.h"
#include "Callbacks/CbThread.h"
#include "Callbacks/CbImage.h"

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeMonitor(
	PDRIVER_OBJECT DriverObject
) {
	NTSTATUS status;

	UNREFERENCED_PARAMETER(DriverObject);

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
}
