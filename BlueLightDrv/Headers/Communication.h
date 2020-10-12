#pragma once

#include "DeviceAPI.h"

NTSTATUS BlCreateCommumicationPort(
	_In_ PSECURITY_DESCRIPTOR SecurityDescriptor
);

NTSTATUS BlSendMessage(
	_In_ PVOID Message,
	_In_ ULONG MessageSize
);