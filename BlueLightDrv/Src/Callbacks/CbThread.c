#include <ntddk.h>

#include "Callbacks/CbThread.h"

#include "Helper.h"
#include "DeviceAPI.h"
#include "Communication.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

BOOLEAN g_PsSetCreateThreadNotifyRoutineCreated = FALSE;

////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

/*
* When a thread is created, the thread-notify routine runs in the context of the thread that created the new thread. 
* When a thread is deleted, the thread-notify routine runs in the context of this thread when the thread exits.
* 
* When in this routine, target thread has been created and is in the INITIALIZED state. 
* It will not transition to READY until this routine exits.
* 
* CreateRemoteThread - ProcessId != GetCurrentProcessId && not the first thread in the process.
* 
* Available starting with Windows 2000.
*/
VOID BlCreateThreadNotifyCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
);

VOID ThreadLogger(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
);

VOID CreateRemoteThreadLogger(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
);

VOID SendThreadCreateExitNotification(
	_In_ HANDLE ProcessId,
	_In_ HANDLE ThreadId,
	_In_ BOOLEAN Create
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

// TODO Check for OS Version. Ref: https://github.com/OSRDrivers/kmexts
NTSTATUS InitializeThreadCallbacks() {
	NTSTATUS status;

	status = PsSetCreateThreadNotifyRoutine(BlCreateThreadNotifyCallback);
	if (!NT_SUCCESS(status)) {
		LogError("Error, process notify registartion failed with code:%08x", status);
		DestroyThreadCallbacks();
		return status;
	}
	g_PsSetCreateThreadNotifyRoutineCreated = TRUE;

	LogTrace("Thread callbacks initialization is completed");
	return STATUS_SUCCESS;
}

VOID DestroyThreadCallbacks() {
	if (g_PsSetCreateThreadNotifyRoutineCreated)
		PsRemoveCreateThreadNotifyRoutine(BlCreateThreadNotifyCallback);

	LogTrace("Thread callbacks deinitialization is completed");
}

////////////////////////////////////////////////
// Thread Callbacks
////////////////////////////////////////////////

//
// The callback functions will call for action functions
// accordding to what action we want to preform (Log, StartInjection, SendEvents...)
//

VOID BlCreateThreadNotifyCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
) {
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ThreadId);
	UNREFERENCED_PARAMETER(Create);

	//ThreadLogger(ProcessId, ThreadId, Create);
	//CreateRemoteThreadLogger(ProcessId, ThreadId, Create);

	SendThreadCreateExitNotification(ProcessId, ThreadId, Create);

	return;
}

////////////////////////////////////////////////
// Action Rutines
////////////////////////////////////////////////

VOID ThreadLogger(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
) {
	LogInfo("Thread: %u %s in Process: %u by Process: %u",
		HandleToULong(ThreadId),
		Create ? "being created" : "exiting",
		HandleToULong(ProcessId),
		HandleToULong(PsGetCurrentProcessId())
	);
}

VOID CreateRemoteThreadLogger(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
) {
	if (PsGetCurrentProcessId() != ProcessId) {
		LogTrace("Possible CreateRemoteThread: %u %s in Process: %u by Process: %u",
			HandleToULong(ThreadId),
			Create ? "being created" : "exiting",
			HandleToULong(ProcessId),
			HandleToULong(PsGetCurrentProcessId())
		);
	}
}

VOID SendThreadCreateExitNotification(
	_In_ HANDLE ProcessId,
	_In_ HANDLE ThreadId,
	_In_ BOOLEAN Create
) {
	Bl_ThreadPacket message = { 0 };

	// Header
	KeQuerySystemTimePrecise(&message.Header.time);
	message.Header.type = Create ? BlThreadCreate : BlThreadExit;
	message.Header.size = sizeof(Bl_ThreadPacket);

	// Data
	message.ProcessId = HandleToULong(ProcessId);
	message.ThreadId = HandleToULong(ThreadId);

	BlSendMessage(&message, message.Header.size);
}