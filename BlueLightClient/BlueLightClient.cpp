#include <iostream>
#include <Windows.h>
#include <fltUser.h>

#include "../BlueLightDrv/Headers/DeviceAPI.h"

void DisplayTime(const LARGE_INTEGER& time) {
    SYSTEMTIME st;
    ::FileTimeToSystemTime((FILETIME*)&time, &st);
    printf("%02d:%02d:%02d.%03d: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void HandleMessage(const BYTE* buffer) {
    auto header = (PBl_EventPacketHeader)buffer;
    printf("type = %lu\n", buffer[0]);
    printf("size = %lu\n", *((ULONG*)buffer + sizeof(Bl_EventType)));
    printf("pid = %lu\n", *((ULONG*)buffer + sizeof(Bl_EventPacketHeader)));

    DisplayTime(header->creationTime);

    printf("buffer: %s\n", buffer);

    if (header->type == ProcessCreate) {
        auto info = (PBl_ProcessCreatePacket)buffer;
        printf("Process %d created.\n", info->ProcessId);
    }
}

int main() {
    HANDLE hPort;

    HRESULT hResult = FilterConnectCommunicationPort(
        DEVICE_COMM_PORT,
        0,
        nullptr,
        0,
        nullptr,
        &hPort
    );
    if (FAILED(hResult)) {
        printf("Error connecting to port (HR=0x%08X)\n", hResult);
        return 1;
    }

    BYTE buffer[1 << 12];
    FILTER_MESSAGE_HEADER* message = (FILTER_MESSAGE_HEADER*)buffer;

    while (true) {
        hResult = FilterGetMessage(
            hPort,
            message,
            sizeof(buffer),
            nullptr
        );
        if (FAILED(hResult)) {
            printf("Error receiving message (0x%08X)\n", hResult);
            break;
        }

        HandleMessage(buffer + sizeof(FILTER_MESSAGE_HEADER));
    }
}