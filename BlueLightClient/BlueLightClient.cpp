#include <iostream>
#include <iomanip>
#include <string>

#include <Windows.h>
#include <fltUser.h>

#include "../BlueLightDrv/Headers/DeviceAPI.h"

void DisplayTime(const LARGE_INTEGER& time) {
    SYSTEMTIME st;
    ::FileTimeToSystemTime((FILETIME*)&time, &st);
    printf("%02d:%02d:%02d.%03d: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

void PrintProcessCreate(PBl_ProcessCreatePacket item) {
    std::wstring fileName((WCHAR*)((UCHAR*)item + item->FileNameOffset), item->FileNameLength);
    std::wstring commandLine((WCHAR*)((UCHAR*)item + item->CommandLineOffset), item->CommandLineLength);

    printf("[%d] %ws created.\n", item->ProcessId, fileName.c_str());
    printf("===========================\n");

    printf("\tParentProcessId = %d\n", item->ParentProcessId);
    printf("\tCommandLine = %ws\n", commandLine.c_str());
    
    printf("\n");
}

void PrintProcessExit(PBl_ProcessExitPacket item) {
    printf("[%d] exited.\n", item->ProcessId);
    printf("\n");
}

void PrintThreadCreateExit(PBl_ThreadPacket item) {
    printf("ThreadId [%d] %s. ProcessId = [%d]\n",
        item->ThreadId,
        item->Header.type == BlThreadCreate ? "created" : "exited",
        item->ProcessId
    );
    printf("\n");
}

void PrintLoadImage(PBl_LoadImagePacket item) {
    std::wstring imageName((WCHAR*)((UCHAR*)item + item->ImageNameOffset), item->ImageNameLength);

    printf("[%d] loaded image: \n", item->ProcessId);
    printf("===========================\n");

    printf("\tImage Name = %ws\n", imageName.c_str());
    printf("\tImage Base = %p\n", item->ImageBaseAddress);

    printf("\n");
}

void HandleMessage(const BYTE* buffer) {
    auto header = (PBl_EventPacketHeader)buffer;

    DisplayTime(header->time);
    printf("EventType = %d, ", header->type);
    printf("Pakcet Size = %lu\n", header->size);

    if (header->type == BlProcessCreate) {
        PrintProcessCreate((PBl_ProcessCreatePacket)buffer);
    }
    else if (header->type == BlProcessExit) {
        PrintProcessExit((PBl_ProcessExitPacket)buffer);
    }
    else if (header->type == BlThreadCreate || header->type == BlThreadExit) {
        PrintThreadCreateExit((PBl_ThreadPacket)buffer);
    }
    else if (header->type == BlLoadImage) {
        PrintLoadImage((PBl_LoadImagePacket)buffer);
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