#pragma once

#include <ntddk.h>

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45,
	SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG           NextEntryOffset;
	ULONG           NumberOfThreads;
	LARGE_INTEGER   Reserved[3];
	LARGE_INTEGER   CreateTime;
	LARGE_INTEGER   UserTime;
	LARGE_INTEGER   KernelTime;
	UNICODE_STRING  ImageName;
	KPRIORITY       BasePriority;
	HANDLE          ProcessId;
	HANDLE          InheritedFromProcessId;
	ULONG           HandleCount;
	UCHAR           Reserved4[4];
	PVOID           Reserved5[11];
	SIZE_T          PeakPagefileUsage;
	SIZE_T          PrivatePageCount;
	LARGE_INTEGER   Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

NTSYSAPI NTSTATUS NTAPI ZwQueryInformationProcess(
	_In_      HANDLE                    ProcessHandle,
	_In_      PROCESSINFOCLASS          ProcessInformationClass,
	_Out_     PVOID                     ProcessInformation,
	_In_      ULONG                     ProcessInformationLength,
	_Out_opt_ PULONG                    ReturnLength
);

NTKERNELAPI
PCHAR
NTAPI
PsGetProcessImageFileName(
	_In_ PEPROCESS Process
);