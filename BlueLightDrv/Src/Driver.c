#include <fltKernel.h>
#include <ntddk.h>

#include "Driver.h"
#include "Device.h"
#include "Configs.h"
#include "Helper.h"
#include "FsMiniFilter.h"
#include "Monitor.h"
#include "Injector.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

#define DRIVER_TAG 'lb' // bl

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

PDRIVER_OBJECT g_DriverObject = NULL;
volatile LONG g_DriverActive = FALSE;

////////////////////////////////////////////////
// Load / Unload
////////////////////////////////////////////////

_Function_class_(DRIVER_UNLOAD)
VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	DestroyMonitor();
	DestroyFsMiniFilter();
	DestroyDevice();

	LogTrace("Driver has been unloaded");
}

_Function_class_(DRIVER_INITIALIZE)
NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
) {
	NTSTATUS status;

	SetDriverState(TRUE);

	status = InitializeConfigs(RegistryPath);
	if (!NT_SUCCESS(status)) {
		LogWarning("Error, can't initialize configs");
	}
	
	status = InitializeMonitor(DriverObject);
	if (!NT_SUCCESS(status)) {
		LogWarning("Error, object monitor haven't started");
	}

	status = InitializeFsMiniFilter(DriverObject);
	if (!NT_SUCCESS(status)) {
		LogWarning("Error, file-system mini-filter haven't started");
	}

	status = InitializeDevice(DriverObject);
	if (!NT_SUCCESS(status)) {
		LogWarning("Error, can't create device");
	}
	
	status = InitializeInjector(DriverObject, RegistryPath);
	if (!NT_SUCCESS(status)) {
		LogWarning("Error, can't initialize injector");
	}

	DestroyConfigs();

	DriverObject->DriverUnload = DriverUnload;
	g_DriverObject = DriverObject;

	LogTrace("Driver has been created");

	return STATUS_SUCCESS;
}

////////////////////////////////////////////////
// Utils
////////////////////////////////////////////////

VOID SetDriverState(BOOLEAN activate) {
	InterlockedExchange(&g_DriverActive, (LONG)activate);
}

BOOLEAN IsDriverActive() {
	return (g_DriverActive ? TRUE : FALSE);
}