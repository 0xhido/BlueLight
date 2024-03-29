#include <fltKernel.h>
#include <dontuse.h>

#include "../../../injlib/injlib.h"

#include "Callbacks/CbProcess.h"

#include "Helper.h"
#include "DeviceAPI.h"
#include "FsMiniFilter.h"
#include "Communication.h"
#include "Undocumented.h"
#include "Data/ProcessTable.h"

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
	_In_ HANDLE ProcessId,
	_In_ PPS_CREATE_NOTIFY_INFO CreateInfo
);

VOID SendProcessExitNotification(
	_In_ HANDLE ProcessId
);

VOID UpdateProcessTable(
	_In_ HANDLE ProcessId,
	_In_ PPS_CREATE_NOTIFY_INFO CreateInfo
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
	UpdateProcessTable(ProcessId, CreateInfo);
	
	if (Globals.BlClientPort) {
		if (CreateInfo) {
			SendProcessCreateNotification(ProcessId, CreateInfo);
		}
		else {
			SendProcessExitNotification(ProcessId);
		}
	}

	InjCreateProcessNotifyRoutineEx(Process, ProcessId, CreateInfo);

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
	_In_ HANDLE ProcessId,
	_In_ PPS_CREATE_NOTIFY_INFO CreateInfo
) {
	PBl_ProcessCreatePacket message;
	ULONG messageSize = sizeof(Bl_ProcessCreatePacket);
	ULONG fileNameSize = 0;
	ULONG commandLineSize = 0;

	if (CreateInfo->ImageFileName) {
		fileNameSize = CreateInfo->ImageFileName->Length;
		messageSize += fileNameSize;
	}

	if (CreateInfo->CommandLine) {
		commandLineSize = CreateInfo->CommandLine->Length;
		messageSize += commandLineSize;
	}

	message = (PBl_ProcessCreatePacket)ExAllocatePoolWithTag(NonPagedPool, messageSize, PROCESS_CREATE_TAG);
	if (!message) {
		LogWarning("Could not allocate memory for process creation message");
		return;
	}

	// Header
	KeQuerySystemTimePrecise(&message->Header.time);
	message->Header.type = BlProcessCreate;
	message->Header.size = messageSize;

	// Data
	message->ProcessId = HandleToULong(ProcessId);
	message->ParentProcessId = HandleToULong(CreateInfo->ParentProcessId);

	message->FileNameLength = 0;
	message->FileNameOffset = sizeof(Bl_ProcessCreatePacket);
	if (fileNameSize > 0) {
		memcpy_s(
			(UCHAR*)message + message->FileNameOffset,
			fileNameSize,
			CreateInfo->ImageFileName->Buffer,
			fileNameSize
		);
		message->FileNameLength = fileNameSize / sizeof(WCHAR);
	}

	message->CommandLineLength = 0;
	message->CommandLineOffset = message->FileNameOffset + fileNameSize;
	if (commandLineSize > 0) {
		memcpy_s(
			(UCHAR*)message + message->CommandLineOffset,
			commandLineSize,
			CreateInfo->CommandLine->Buffer,
			commandLineSize
		);
		message->CommandLineLength = commandLineSize / sizeof(WCHAR);
	}

	BlSendMessage(message, message->Header.size);
	
	ExFreePoolWithTag(message, PROCESS_CREATE_TAG);
}

VOID SendProcessExitNotification(
	_In_ HANDLE ProcessId
) {
	Bl_ProcessExitPacket message = { 0 };

	// Header
	KeQuerySystemTimePrecise(&message.Header.time);
	message.Header.type = BlProcessExit;
	message.Header.size = sizeof(Bl_ProcessExitPacket);

	// Data
	message.ProcessId = HandleToULong(ProcessId);

	BlSendMessage(&message, message.Header.size);
}

VOID UpdateProcessTable(
	_In_ HANDLE ProcessId,
	_In_ PPS_CREATE_NOTIFY_INFO CreateInfo
) {
	ProcessTableEntry entry;
	BOOLEAN isInTable; 

	entry.ProcessId = ProcessId;

	ExAcquireFastMutex(&g_ProcessTableLock);
	
	isInTable = GetProcessInProcessTable(&entry);
	
	if (CreateInfo && !isInTable) { 
		entry.ParentProcessId = CreateInfo->ParentProcessId;
		entry.IsInjected = FALSE;
		entry.IsFirstThread = TRUE;

		AddProcessToProcessTable(&entry);
	}
	else if (!CreateInfo && isInTable) { 
		RemoveProcessFromProcessTable(&entry);
	}

	ExReleaseFastMutex(&g_ProcessTableLock);
}