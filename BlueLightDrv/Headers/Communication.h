#pragma once

#include "DeviceAPI.h"

NTSTATUS BlCreateCommumicationPort(
	_In_ PSECURITY_DESCRIPTOR SecurityDescriptor
);

NTSTATUS BlSendMessage(
	_In_ PBl_EventPacketHeader EventPacket
);