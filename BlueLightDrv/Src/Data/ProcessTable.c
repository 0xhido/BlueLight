#include <ntddk.h>

#include "Data/ProcessTable.h"

#include "Helper.h"
#include "Undocumented.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

#define PROCESS_TABLE_TAG 'tpLB' // BLpt - BlueLight Process Table
#define PROCESS_QUERY_LIMITED_INFORMATION 0x1000

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

RTL_AVL_TABLE	g_ProcessTable;

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

_Function_class_(RTL_AVL_COMPARE_ROUTINE)
RTL_GENERIC_COMPARE_RESULTS CompareProcessTableEntry(
	struct _RTL_AVL_TABLE* Table,
	PVOID  FirstStruct,
	PVOID  SecondStruct
);

_Function_class_(RTL_AVL_ALLOCATE_ROUTINE)
PVOID AllocateProcessTableEntry(
	struct _RTL_AVL_TABLE* Table,
	CLONG  ByteSize
);

_Function_class_(RTL_AVL_FREE_ROUTINE)
VOID FreeProcessTableEntry(
	struct _RTL_AVL_TABLE* Table,
	PVOID  Buffer
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeProcessTable() {
	PSYSTEM_PROCESS_INFORMATION first, processInfo = NULL;
	NTSTATUS status = STATUS_SUCCESS;
	SIZE_T size = 0, offset;

	// Initialize process table
	RtlInitializeGenericTableAvl(
		&g_ProcessTable,
		CompareProcessTableEntry,
		AllocateProcessTableEntry,
		FreeProcessTableEntry,
		NULL
	);

	// Initialize process table lock
	ExInitializeFastMutex(&g_ProcessTableLock);

	// Get all current running processes
	status = QuerySystemInformation(
		SystemProcessInformation,
		&processInfo,
		&size
	);
	if (!NT_SUCCESS(status)) {
		LogError("Error, query system information(pslist) failed with code:%08x", status);
		return status;
	}

	offset = 0;
	first = processInfo;

	do {
		ProcessTableEntry entry;

		processInfo = (PSYSTEM_PROCESS_INFORMATION)((SIZE_T)processInfo + offset);
		
		if (processInfo->ProcessId == 0) {
			offset = processInfo->NextEntryOffset;
			continue;
		}

		RtlZeroMemory(&entry, sizeof(entry));

		entry.ProcessId = processInfo->ProcessId;
		entry.ParentProcessId = processInfo->InheritedFromProcessId;

		entry.IsFirstThread = FALSE;
		entry.IsInjected = FALSE;

		if (!AddProcessToProcessTable(&entry)) {
			LogWarning("Warning, can't add process(pid:%p) to process table", processInfo->ProcessId);
		}

		offset = processInfo->NextEntryOffset;
	} while (offset);
	
	FreeInformation(first);
	LogTrace("Process table initialization is completed");
	return STATUS_SUCCESS;
}

VOID DestroyProcessTable() {
	PProcessTableEntry entry;
	PVOID restartKey = NULL;

	for (entry = RtlEnumerateGenericTableWithoutSplayingAvl(&g_ProcessTable, &restartKey);
		entry != NULL;
		entry = RtlEnumerateGenericTableWithoutSplayingAvl(&g_ProcessTable, &restartKey))
	{
		if (!RemoveProcessFromProcessTable(entry))
			LogWarning("Warning, can't remove element from process table, looks like memory leak");

		restartKey = NULL; // reset enum
	}
	LogTrace("Process table deinitialization is completed");
}

////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////

_Function_class_(RTL_AVL_COMPARE_ROUTINE)
RTL_GENERIC_COMPARE_RESULTS CompareProcessTableEntry(
	struct _RTL_AVL_TABLE* Table, 
	PVOID  FirstStruct, 
	PVOID  SecondStruct
) {
	PProcessTableEntry first = (PProcessTableEntry)FirstStruct;
	PProcessTableEntry second = (PProcessTableEntry)SecondStruct;

	UNREFERENCED_PARAMETER(Table);

	if (first->ProcessId > second->ProcessId)
		return GenericGreaterThan;

	if (first->ProcessId < second->ProcessId)
		return GenericLessThan;

	return GenericEqual;
}

_Function_class_(RTL_AVL_ALLOCATE_ROUTINE)
PVOID AllocateProcessTableEntry(
	struct _RTL_AVL_TABLE* Table, 
	CLONG  ByteSize
) {
	UNREFERENCED_PARAMETER(Table);
	return ExAllocatePoolWithTag(NonPagedPool, ByteSize, PROCESS_TABLE_TAG);
}

_Function_class_(RTL_AVL_FREE_ROUTINE)
VOID FreeProcessTableEntry(
	struct _RTL_AVL_TABLE* Table, 
	PVOID  Buffer
) {
	UNREFERENCED_PARAMETER(Table);
	ExFreePoolWithTag(Buffer, PROCESS_TABLE_TAG);
}


////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////


BOOLEAN AddProcessToProcessTable(PProcessTableEntry entry) {
	BOOLEAN result = FALSE;

	if (RtlInsertElementGenericTableAvl(&g_ProcessTable, entry, sizeof(ProcessTableEntry), &result) == NULL)
		return FALSE;

	return result;
}

BOOLEAN RemoveProcessFromProcessTable(PProcessTableEntry entry) {
	return RtlDeleteElementGenericTableAvl(&g_ProcessTable, entry);
}

BOOLEAN GetProcessInProcessTable(PProcessTableEntry entry) {
	PProcessTableEntry entry2;

	entry2 = (PProcessTableEntry)RtlLookupElementGenericTableAvl(&g_ProcessTable, entry);
	if (entry2)
		RtlCopyMemory(entry, entry2, sizeof(ProcessTableEntry));

	return (entry2 ? TRUE : FALSE);
}

BOOLEAN UpdateProcessInProcessTable(PProcessTableEntry entry) {
	PProcessTableEntry entry2;

	entry2 = (PProcessTableEntry)RtlLookupElementGenericTableAvl(&g_ProcessTable, entry);

	if (entry2)
		RtlCopyMemory(entry2, entry, sizeof(ProcessTableEntry));

	return (entry2 ? TRUE : FALSE);
}