#include "Helper.h"
#include "Configs.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

#define CONFIG_TAG 'fclb' // blcf - BlueLight Config

////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////

typedef struct _BlConfigContext {
	BOOLEAN state;
	UNICODE_STRING ignoreImages;
} BlConfigContext, * PBlConfigContext;

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

PBlConfigContext g_ConfigContext = NULL;

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

NTSTATUS InitializeConfigs(
	PUNICODE_STRING RegistryPath
);

NTSTATUS DestroyConfigs();

NTSTATUS QueryAndAllocRegistryData(
	HANDLE hKey,
	LPCWSTR Value,
	ULONG Type,
	PUNICODE_STRING Data,
	PUNICODE_STRING Default
);

VOID ReleaseRegistryData(
	PUNICODE_STRING Data
);

VOID ReleaseConfigContext(
	PBlConfigContext context
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

NTSTATUS InitializeConfigs(
	PUNICODE_STRING RegistryPath
) {
	BlConfigContext config;
	OBJECT_ATTRIBUTES attribs;
	NTSTATUS status;
	HANDLE hKey;

	if (g_ConfigContext) {
		return STATUS_ALREADY_INITIALIZED;
	}

	RtlZeroMemory(&config, sizeof(config));

	InitializeObjectAttributes(&attribs, RegistryPath, 0, NULL, NULL);

	status = ZwOpenKey(&hKey, KEY_ALL_ACCESS, &attribs);
	if (!NT_SUCCESS(status)) {
		LogError("Error, can't open config registry key, code: %08x", status);
		return status;
	}

	QueryAndAllocRegistryData(hKey, L"Bl_IgnoredImages", REG_MULTI_SZ, &config.ignoreImages, NULL);

	g_ConfigContext = (PBlConfigContext)ExAllocatePoolWithTag(NonPagedPool, sizeof(config), CONFIG_TAG);
	if (!g_ConfigContext)
	{
		LogError("Error, can't allocate memory for the config context");
		ReleaseConfigContext(&config);
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(g_ConfigContext, &config, sizeof(config));

	LogTrace("Config is initialized");
	return STATUS_SUCCESS;
}

NTSTATUS QueryAndAllocRegistryData(
	HANDLE hKey, 
	LPCWSTR Value, 
	ULONG Type, 
	PUNICODE_STRING Data, 
	PUNICODE_STRING Default
) {
	PKEY_VALUE_PARTIAL_INFORMATION info = NULL;
	UNICODE_STRING valueName;
	ULONG length, dataLength;
	NTSTATUS status;
	PVOID dataBuffer;

	if (Default) {
		dataLength = Default->Length;
		dataBuffer = ExAllocatePoolWithTag(NonPagedPool, dataLength, CONFIG_TAG);
		if (!dataBuffer) {
			return STATUS_NO_MEMORY;
		}

		RtlCopyMemory(dataBuffer, Default->Buffer, dataLength);
	}
	else {
		dataLength = 0;
		dataBuffer = NULL;
	}

	RtlInitUnicodeString(&valueName, Value);

	status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, NULL, 0, &length);
	if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL)
		goto end_proc;

	if (length < sizeof(KEY_VALUE_PARTIAL_INFORMATION))
		goto end_proc;

	info = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, length, CONFIG_TAG);
	if (!info)
		goto end_proc;

	status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, info, length, &length);
	if (!NT_SUCCESS(status))
		goto end_proc;

	if (info->Type != Type)
		goto end_proc;

	if (info->DataLength == 0 || info->DataLength > 0xFFFF)
		goto end_proc;

	if (dataBuffer)
		ExFreePoolWithTag(dataBuffer, CONFIG_TAG);

	dataLength = info->DataLength;
	dataBuffer = ExAllocatePoolWithTag(NonPagedPool, dataLength, CONFIG_TAG);
	if (!dataBuffer)
	{
		ExFreePoolWithTag(info, CONFIG_TAG);
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(dataBuffer, info->Data, dataLength);

end_proc:

	if (info)
		ExFreePoolWithTag(info, CONFIG_TAG);

	Data->Buffer = (PWCH)dataBuffer;
	Data->Length = (USHORT)dataLength;
	Data->MaximumLength = (USHORT)dataLength;

	return STATUS_SUCCESS;
}

NTSTATUS DestroyConfigs()
{
	if (!g_ConfigContext)
		return STATUS_NOT_FOUND;

	ReleaseConfigContext(g_ConfigContext);
	ExFreePoolWithTag(g_ConfigContext, CONFIG_TAG);

	LogTrace("Config is destroyed");
	return STATUS_SUCCESS;
}

////////////////////////////////////////////////
// Utils
////////////////////////////////////////////////

VOID ReleaseRegistryData(
	PUNICODE_STRING Data
) {
	if (Data->Length)
		ExFreePoolWithTag(Data->Buffer, CONFIG_TAG);
}

VOID ReleaseConfigContext(
	PBlConfigContext context
) {
	ReleaseRegistryData(&context->ignoreImages);
}
