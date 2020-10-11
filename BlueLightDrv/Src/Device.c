#include "Device.h"
#include "DeviceAPI.h"
#include "Driver.h"
#include "Helper.h"

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

BOOLEAN g_DeviceCreated = FALSE;
PDEVICE_OBJECT g_DeviceObject = NULL;

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeDevice(
	PDRIVER_OBJECT DriverObject
) {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(DEVICE_NAME);
	UNICODE_STRING dosDeviceName = RTL_CONSTANT_STRING(DOS_DEVICES_LINK_NAME);
	PDEVICE_OBJECT deviceObject = NULL;

	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
	if (!NT_SUCCESS(status))
	{
		LogError("Error, device creation failed with code:%08x", status);
		return status;
	}

	status = IoCreateSymbolicLink(&dosDeviceName, &deviceName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(deviceObject);
		LogError("Error, symbolic link creation failed with code:%08x", status);
		return status;
	}

	DriverObject->MajorFunction[IRP_MJ_CREATE] = IrpDeviceCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = IrpDeviceClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = IrpDeviceCleanup;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpDeviceControlHandler;
	g_DeviceObject = deviceObject;
	g_DeviceCreated = TRUE;

	LogTrace("Device initialization is completed");
	return status;
}

NTSTATUS DestroyDevice() {
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING dosDeviceName = RTL_CONSTANT_STRING(DOS_DEVICES_LINK_NAME);

	if (!g_DeviceObject)
		return STATUS_NOT_FOUND;

	status = IoDeleteSymbolicLink(&dosDeviceName);
	if (!NT_SUCCESS(status))
		LogWarning("Error, symbolic link deletion failed with code:%08x", status);

	IoDeleteDevice(g_DeviceObject);

	g_DeviceCreated = FALSE;

	LogTrace("Device deinitialization is completed");
	return status;
}

////////////////////////////////////////////////
// IRP Major Functions
////////////////////////////////////////////////

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_CREATE)
NTSTATUS IrpDeviceCreate(
	PDEVICE_OBJECT DeviceObject, 
	PIRP Irp
) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_CLOSE)
NTSTATUS IrpDeviceClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_CLEANUP)
NTSTATUS IrpDeviceCleanup(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
) {
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Function_class_(DRIVER_DISPATCH)
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
NTSTATUS IrpDeviceControlHandler(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
) {
	PIO_STACK_LOCATION irpStack;
	Bl_StatusPacket result;
	NTSTATUS status = STATUS_SUCCESS;
	PVOID inputBuffer, outputBuffer, outputData;
	ULONG ioctl, inputBufferSize, outputBufferSize, outputBufferMaxSize,
		outputDataMaxSize, outputDataSize;

	UNREFERENCED_PARAMETER(DeviceObject);

	// Get irp information

	irpStack = IoGetCurrentIrpStackLocation(Irp);
	ioctl = irpStack->Parameters.DeviceIoControl.IoControlCode;

	inputBuffer = outputBuffer = Irp->AssociatedIrp.SystemBuffer;
	inputBufferSize = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBufferMaxSize = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	outputBufferSize = 0;
	outputDataSize = 0;
	outputDataMaxSize = 0;

	RtlZeroMemory(&result, sizeof(result));

	// Check output buffer size

	if (outputBufferMaxSize < sizeof(result))
	{
		status = STATUS_INVALID_PARAMETER;
		goto EndProc;
	}

	// Prepare additional buffer for output data 
	outputData = (PVOID)((UINT_PTR)outputBuffer + sizeof(result));
	outputDataMaxSize = outputBufferMaxSize - sizeof(result);

	// Important limitation:
	// Because both input (inputBuffer) and output data (outputData) are located in the same buffer there is a limitation for the output
	// buffer usage. When a ioctl handler is executing, it can use the input buffer only until first write to the output buffer, because
	// when you put data to the output buffer you can overwrite data in input buffer. Therefore if you gonna use both an input and output 
	// data in the same time you should make the copy of input data and work with it.
	switch (ioctl)
	{
	default:
		LogWarning("Unknown IOCTL code:%08x", ioctl);
		status = STATUS_INVALID_PARAMETER;
		goto EndProc;
	}

EndProc:

	// If additional output data has been presented
	if (NT_SUCCESS(status) && outputDataSize > 0)
	{
		if (outputDataSize > outputDataMaxSize)
		{
			LogWarning("An internal error, looks like a stack corruption!");
			outputDataSize = outputDataMaxSize;
			result.status = (ULONG)STATUS_PARTIAL_COPY;
		}

		result.dataSize = outputDataSize;
	}

	// Copy result to output buffer
	if (NT_SUCCESS(status))
	{
		outputBufferSize = sizeof(result) + outputDataSize;
		RtlCopyMemory(outputBuffer, &result, sizeof(result));
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = outputBufferSize;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}