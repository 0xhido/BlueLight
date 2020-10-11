#pragma once

#include <ntddk.h>

NTSTATUS InitializeMonitor(PDRIVER_OBJECT DriverObject);
VOID DestroyMonitor();