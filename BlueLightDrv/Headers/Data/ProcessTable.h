#pragma once

////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////

typedef struct _ProcessTableEntry {
	HANDLE ProcessId;
	HANDLE ParentProcessId;
	BOOLEAN IsFirstThread;
	BOOLEAN IsInjected;
} ProcessTableEntry, * PProcessTableEntry;

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

FAST_MUTEX g_ProcessTableLock;

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeProcessTable();
VOID DestroyProcessTable();

////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////

BOOLEAN AddProcessToProcessTable(PProcessTableEntry entry);
BOOLEAN RemoveProcessFromProcessTable(PProcessTableEntry entry);
BOOLEAN GetProcessInProcessTable(PProcessTableEntry entry);
BOOLEAN UpdateProcessInProcessTable(PProcessTableEntry entry);
