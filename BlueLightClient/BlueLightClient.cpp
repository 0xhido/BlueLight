#include <iostream>
#include <iomanip>

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

    DisplayTime(header->creationTime);
    printf("EventType = %d, ", header->type);
    printf("Pakcet Size = %lu, ", header->size);

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
        NULL,
        0,
        NULL,
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
            NULL
        );
        if (FAILED(hResult)) {
            printf("Error receiving message (0x%08X)\n", hResult);
            break;
        }

        HandleMessage(buffer + sizeof(FILTER_MESSAGE_HEADER));
    }
}