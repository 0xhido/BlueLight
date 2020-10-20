#pragma once

NTSTATUS InitializeDevice(PDRIVER_OBJECT DriverObject);
NTSTATUS DestroyDevice();

////////////////////////////////////////////////
// IRP Major Functions
////////////////////////////////////////////////

NTSTATUS IrpDeviceCreate(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

NTSTATUS IrpDeviceClose(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

NTSTATUS IrpDeviceCleanup(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);

NTSTATUS IrpDeviceControlHandler(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
);