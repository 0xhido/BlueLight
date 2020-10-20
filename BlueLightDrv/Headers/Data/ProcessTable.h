#pragma once

typedef struct _ProcessTableEntry {
	HANDLE ProcessId;
	HANDLE ParentProcessId;
	BOOLEAN IsFirstThread;
	BOOLEAN IsInjected;
} ProcessTableEntry, * PProcessTableEntry;

NTSTATUS InitializeProcessTable();
VOID DestroyProcessTable();

BOOLEAN AddProcessToProcessTable(PProcessTableEntry entry);
BOOLEAN RemoveProcessFromProcessTable(PProcessTableEntry entry);
BOOLEAN GetProcessInProcessTable(PProcessTableEntry entry);
BOOLEAN UpdateProcessInProcessTable(PProcessTableEntry entry);
