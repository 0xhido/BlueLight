#pragma once

NTSTATUS InitializeInjector(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);
VOID DestroyInjector();
