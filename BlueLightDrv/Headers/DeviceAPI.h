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
	BlNone,
	BlProcessCreate,
	BlProcessExit,
	BlThreadCreate,
	BlThreadExit,
	BlLoadImage,
	BlRemoteThread
} Bl_EventType;

typedef struct _Bl_EventPacketHeader {
	Bl_EventType type;
	ULONG size;
	LARGE_INTEGER time;
} Bl_EventPacketHeader, * PBl_EventPacketHeader;

typedef struct _Bl_ProcessCreatePacket {
	Bl_EventPacketHeader Header;
	ULONG ProcessId;
	ULONG ParentProcessId;
	ULONG FileNameLength;
	ULONG FileNameOffset;
	ULONG CommandLineLength;
	ULONG CommandLineOffset;
} Bl_ProcessCreatePacket, * PBl_ProcessCreatePacket;

typedef struct _Bl_ProcessExitPacket {
	Bl_EventPacketHeader Header;
	ULONG ProcessId;
} Bl_ProcessExitPacket, * PBl_ProcessExitPacket;

typedef struct _Bl_ThreadPacket {
	Bl_EventPacketHeader Header;
	ULONG ProcessId;
	ULONG ThreadId;
} Bl_ThreadPacket, * PBl_ThreadPacket;

typedef struct _Bl_LoadImagePacket {
	Bl_EventPacketHeader Header;
	ULONG ProcessId;
	PVOID ImageBaseAddress;
	ULONG ImageNameLength;
	ULONG ImageNameOffset;
} Bl_LoadImagePacket, * PBl_LoadImagePacket;

// Result packet
typedef struct _Bl_StatusPacket {
	unsigned int status;
	unsigned int dataSize;
	union {
		unsigned long long id;
		unsigned long state;
	} info;
}  Bl_StatusPacket, * PBl_StatusPacket;
