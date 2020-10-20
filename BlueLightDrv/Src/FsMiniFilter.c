#include <fltKernel.h>
#include <dontuse.h>

#include "FsMiniFilter.h"

#include "Helper.h"
#include "Driver.h"
#include "Configs.h"
#include "Communication.h"

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

NTSTATUS FilterSetup(
	PCFLT_RELATED_OBJECTS FltObjects, 
	FLT_INSTANCE_SETUP_FLAGS Flags, 
	DEVICE_TYPE VolumeDeviceType, 
	FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

NTSTATUS FsMiniFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
);

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

BOOLEAN g_FsMiniFilterCreated = FALSE;

////////////////////////////////////////////////
// Minifilter Configuration
////////////////////////////////////////////////

const FLT_CONTEXT_REGISTRATION Contexts[] = {
	{ FLT_CONTEXT_END }
};

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
	{ IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION), //  Size
	FLT_REGISTRATION_VERSION, //  Version
	FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP,                        //  Flags
	Contexts,                 //  Context
	Callbacks,                //  Operation callbacks
	FsMiniFilterUnload,     //  MiniFilterUnload
	FilterSetup,              //  InstanceSetup
	NULL,                     //  InstanceQueryTeardown
	NULL,                     //  InstanceTeardownStart
	NULL,                     //  InstanceTeardownComplete
	NULL,                     //  GenerateFileName
	NULL,                     //  GenerateDestinationFileName
	NULL                      //  NormalizeNameComponent
};

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeFsMiniFilter(
	PDRIVER_OBJECT DriverObject
) {
	NTSTATUS status = STATUS_SUCCESS;
	PSECURITY_DESCRIPTOR sd = NULL;

	// Initialize Global data
	RtlZeroMemory(&Globals, sizeof(Globals));

	try {
		status = FltRegisterFilter(DriverObject, &FilterRegistration, &Globals.Filter);
		if (!NT_SUCCESS(status))
		{
			LogError("Error, can't register filter, code:%08x", status);
			return status;
		}

		status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
		if (!NT_SUCCESS(status)) {
			LogError("Error, can't build default security descriptor, code:%08x", status);
			leave;
		}

		status = BlCreateCommumicationPort(sd);
		if (!NT_SUCCESS(status)) {
			LogError("Error, can't create communication port, code:%08x", status);
			leave;
		}

		status = FltStartFiltering(Globals.Filter);
		if (!NT_SUCCESS(status))
		{
			LogError("Error, can't start filtering, code:%08x", status);
			leave;
		}

		g_FsMiniFilterCreated = TRUE;
	}
	finally {
		if (sd != NULL) {
			FltFreeSecurityDescriptor(sd);
		}

		if (!NT_SUCCESS(status)) {
			if (Globals.BlServerPort != NULL) {
				FltCloseCommunicationPort(Globals.BlServerPort);
			}

			if (Globals.Filter != NULL) {
				FltUnregisterFilter(Globals.Filter);
				Globals.Filter = NULL;
			}
		}
	}

	return status;
}

NTSTATUS DestroyFsMiniFilter() {
	if (!g_FsMiniFilterCreated)
		return STATUS_NOT_FOUND;

	FltCloseCommunicationPort(Globals.BlServerPort);
	Globals.BlServerPort = NULL;

	FltUnregisterFilter(Globals.Filter);
	Globals.Filter = NULL;

	g_FsMiniFilterCreated = FALSE;

	LogTrace("Mini-Filter deitialization is completed");
	return STATUS_SUCCESS;
}

////////////////////////////////////////////////
// Private Functions 
////////////////////////////////////////////////

NTSTATUS FilterSetup(
	PCFLT_RELATED_OBJECTS FltObjects, 
	FLT_INSTANCE_SETUP_FLAGS Flags, 
	DEVICE_TYPE VolumeDeviceType, 
	FLT_FILESYSTEM_TYPE VolumeFilesystemType
) {
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(VolumeDeviceType);
	UNREFERENCED_PARAMETER(VolumeFilesystemType);

	LogTrace("Attach to a new device (flags:%x, device:%d, fs:%d)", (ULONG)Flags, (ULONG)VolumeDeviceType, (ULONG)VolumeFilesystemType);

	return STATUS_SUCCESS;
}

NTSTATUS FsMiniFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
) {
	UNREFERENCED_PARAMETER(Flags);

	if (!g_FsMiniFilterCreated)
		return STATUS_NOT_FOUND;

	FltUnregisterFilter(Globals.Filter);
	Globals.Filter = NULL;
	g_FsMiniFilterCreated = FALSE;

	LogTrace("Mini-Filter unloaded");
	return STATUS_SUCCESS;
}