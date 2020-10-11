#include <fltKernel.h>
#include <dontuse.h>

#include "Callbacks/CbProcess.h"

#include "Helper.h"
#include "DeviceAPI.h"
#include "FsMiniFilter.h"
#include "Communication.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

BOOLEAN g_PsSetCreateProcessNotifyRoutineExCreated = FALSE;

////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

/* 
* When a process is created, the process-notify routine runs in the context of the thread that created the new process. 
* When a process is deleted, the process-notify routine runs in the context of the last thread to exit from the process.
* Available starting with Windows Vista with SP1 and Windows Server 2008.
*/
VOID BlCreateProcessNotifyCallbackEx(
	PEPROCESS Process,
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

VOID ProcessLogger(
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
);

VOID SendProcessCreateNotification(
	_In_ HANDLE ProcessId
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

// TODO Check for OS Version. Ref: https://github.com/OSRDrivers/kmexts
NTSTATUS InitializeProcessCallbacks() {
	NTSTATUS status;

	status = PsSetCreateProcessNotifyRoutineEx(BlCreateProcessNotifyCallbackEx, FALSE);
	if (!NT_SUCCESS(status)) {
		LogError("Error, process notify registartion failed with code:%08x", status);
		DestroyProcessCallbacks();
		return status;
	}
	g_PsSetCreateProcessNotifyRoutineExCreated = TRUE;

	LogTrace("Process callbacks initialization is completed");
	return STATUS_SUCCESS;
}

VOID DestroyProcessCallbacks() {
	if (g_PsSetCreateProcessNotifyRoutineExCreated)
		PsSetCreateProcessNotifyRoutineEx(BlCreateProcessNotifyCallbackEx, TRUE);

	LogTrace("Process callbacks deinitialization is completed");
}

////////////////////////////////////////////////
// Process Callbacks
////////////////////////////////////////////////

//
// The callback functions will call for action functions
// accordding to what action we want to preform (Log, StartInjection, SendEvents...)
//

VOID BlCreateProcessNotifyCallbackEx(
	PEPROCESS Process, 
	HANDLE ProcessId, 
	PPS_CREATE_NOTIFY_INFO CreateInfo
) {
	UNREFERENCED_PARAMETER(Process);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(CreateInfo);

	ProcessLogger(ProcessId, CreateInfo);
	
	if (CreateInfo && Globals.BlClientPort) {
		SendProcessCreateNotification(ProcessId);
	}

	return;
}

////////////////////////////////////////////////
// Action Rutines
////////////////////////////////////////////////

VOID ProcessLogger(
	HANDLE ProcessId,
	PPS_CREATE_NOTIFY_INFO CreateInfo
) {
	if (CreateInfo) {
		LogTrace(
			"Create process, pid:%u, srcPid:%u, srcTid:%u, image:%wZ",
			HandleToULong(ProcessId),
			HandleToULong(PsGetCurrentProcessId()),
			HandleToULong(PsGetCurrentThreadId()),
			CreateInfo->ImageFileName
		);
	}

	else {
		LogTrace(
			"Destroy process, pid:%u, srcPid:%u, srcTid:%u",
			HandleToULong(ProcessId),
			HandleToULong(PsGetCurrentProcessId()),
			HandleToULong(PsGetCurrentThreadId())
		);
	}
}

VOID SendProcessCreateNotification(
	_In_ HANDLE ProcessId
) {
	PBl_ProcessCreatePacket message;

	message = (PBl_ProcessCreatePacket)ExAllocatePoolWithTag(NonPagedPool, sizeof(Bl_ProcessCreatePacket), 'blPc');
	if (!message) {
		LogWarning("Could not allocate memory for process creation message");
		return;
	}

	KeQuerySystemTimePrecise(&message->header.creationTime);
	message->header.type = ProcessCreate;
	message->header.size = sizeof(Bl_ProcessCreatePacket);

	message->ProcessId = HandleToULong(ProcessId);

	BlSendMessage((PBl_EventPacketHeader)message);

	LogInfo("[%u] Message has sent", HandleToULong(ProcessId));
}