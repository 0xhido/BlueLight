#pragma once

////////////////////////////////////////////////
// Device Information
////////////////////////////////////////////////

#define DEVICE_NAME             L"\\Device\\BlueLight"
#define DOS_DEVICES_LINK_NAME   L"\\DosDevices\\BlueLight"
#define DEVICE_WIN32_NAME       L"\\\\.\\BlueLight"

#define DEVICE_COMM_PORT		L"\\BlueLightPort"

////////////////////////////////////////////////
// IOCTL codes
////////////////////////////////////////////////

////////////////////////////////////////////////
// Structs / Packets
////////////////////////////////////////////////

typedef enum _Bl_EventType {
	None,
	ProcessCreate,
	ProcessExit,
	ThreadCreate,
	ThreadExit,
	ImageLoaded,
	RemoteThread
} Bl_EventType;

typedef struct _Bl_EventPacketHeader {
	Bl_EventType type;
	ULONG size;
	LARGE_INTEGER creationTime;
} Bl_EventPacketHeader, * PBl_EventPacketHeader;

typedef struct _Bl_ProcessCreatePacket {
	Bl_EventPacketHeader Header;
	ULONG ProcessId;
} Bl_ProcessCreatePacket, * PBl_ProcessCreatePacket;

// Result packet

typedef struct _Bl_StatusPacket {
	unsigned int status;
	unsigned int dataSize;
	union {
		unsigned long long id;
		unsigned long state;
	} info;
}  Bl_StatusPacket, * PBl_StatusPacket;
