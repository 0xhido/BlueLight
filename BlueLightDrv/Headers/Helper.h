#pragma once

#include <ntddk.h>

#include "Undocumented.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

#define _LogMsg(lvl, lvlname, frmt, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, lvl , "BlueLightDrv!" __FUNCTION__ "[irql:%d,pid:%d][" lvlname "]: " frmt "\n", (ULONG)KeGetCurrentIrql(), HandleToULong(PsGetCurrentProcessId()), __VA_ARGS__)

#define LogError(frmt,   ...) _LogMsg(DPFLTR_ERROR_LEVEL,   "error",   frmt, __VA_ARGS__)
#define LogWarning(frmt, ...) _LogMsg(DPFLTR_WARNING_LEVEL, "warning", frmt, __VA_ARGS__)
#define LogTrace(frmt,   ...) _LogMsg(DPFLTR_TRACE_LEVEL,   "trace",   frmt, __VA_ARGS__)
#define LogInfo(frmt,    ...) _LogMsg(DPFLTR_INFO_LEVEL,    "info",    frmt, __VA_ARGS__)

////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////

NTSTATUS QuerySystemInformation(SYSTEM_INFORMATION_CLASS InfoClass, PVOID* InfoBuffer, PSIZE_T InfoSize);

NTSTATUS QueryProcessInformation(PROCESSINFOCLASS InfoClass, HANDLE Process, PVOID* InfoBuffer, PSIZE_T InfoSize);

PVOID GetProcessInformation(HANDLE hProcess);

VOID FreeInformation(PVOID Buffer);