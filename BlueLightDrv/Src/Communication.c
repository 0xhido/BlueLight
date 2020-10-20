#include <fltKernel.h>
#include <dontuse.h>

#include "Communication.h"

#include "FsMiniFilter.h"
#include "DeviceAPI.h"
#include "Helper.h"

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

NTSTATUS BlConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionPortCookie
);

VOID BlDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
);

NTSTATUS BlMessageNotifyCallback(
	_In_opt_ PVOID PortCookie,
	_In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS BlCreateCommumicationPort(
	_In_ PSECURITY_DESCRIPTOR SecurityDescriptor
) {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING portName = RTL_CONSTANT_STRING(DEVICE_COMM_PORT);
	OBJECT_ATTRIBUTES objectAttributes = { 0 };
	LONG maxConnections = 1;

	InitializeObjectAttributes(
		&objectAttributes,
		&portName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL,
		SecurityDescriptor
	);

	status = FltCreateCommunicationPort(
		Globals.Filter,
		&Globals.BlServerPort,
		&objectAttributes,
		NULL,
		BlConnectNotifyCallback,
		BlDisconnectNotifyCallback,
		BlMessageNotifyCallback,
		maxConnections
	);

	return status;
}

////////////////////////////////////////////////
// Event Callbacks
////////////////////////////////////////////////

NTSTATUS BlConnectNotifyCallback(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionPortCookie
) {
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);

	LogInfo("User has been connected to communication port");
	
	ConnectionPortCookie = NULL;

	Globals.BlClientPort = ClientPort;

	return STATUS_SUCCESS;
}

VOID BlDisconnectNotifyCallback(
	_In_opt_ PVOID ConnectionCookie
) {
	UNREFERENCED_PARAMETER(ConnectionCookie);
	
	LogInfo("User has been disconnected to communication port");

	FltCloseClientPort(Globals.Filter, &Globals.BlClientPort);
	Globals.BlClientPort = NULL;
}

NTSTATUS BlMessageNotifyCallback(
	_In_opt_ PVOID PortCookie,
	_In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength
) {
	UNREFERENCED_PARAMETER(PortCookie);
	UNREFERENCED_PARAMETER(InputBuffer);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(OutputBuffer);
	UNREFERENCED_PARAMETER(OutputBufferLength);

	*ReturnOutputBufferLength = 0;

	LogInfo("User sent message to communication port");

	return STATUS_SUCCESS;
}

NTSTATUS BlSendMessage(
	_In_ PVOID Message,
	_In_ ULONG MessageSize
) {
	NTSTATUS status = STATUS_SUCCESS;
	LONGLONG _1ms = 10000;
	LARGE_INTEGER timeout = { 0 };

	timeout.QuadPart = -500 * _1ms;

	status = FltSendMessage(
		Globals.Filter,
		&Globals.BlClientPort,
		Message,
		MessageSize,
		NULL,
		NULL,
		&timeout
	);

	if (!NT_SUCCESS(status)) {
		LogWarning("FltSendMessage failed. code:%08x", status);
	}

	return status;
}