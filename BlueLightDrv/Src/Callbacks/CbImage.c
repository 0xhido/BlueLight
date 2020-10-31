#include <fltKernel.h>
#include <dontuse.h>

#include "../../../injlib/injlib.h"

#include "Callbacks/CbImage.h"

#include "FsMiniFilter.h"
#include "Helper.h"
#include "DeviceAPI.h"
#include "Communication.h"

////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////

////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////

BOOLEAN g_PsSetLoadImageNotifyRoutineCreated = FALSE;

////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////

////////////////////////////////////////////////
// Declarations
////////////////////////////////////////////////

/*
* When a process is created, the process-notify routine runs in the context of the thread that created the new process.
* When a process is deleted, the process-notify routine runs in the context of the last thread to exit from the process.
* Available starting with Windows Vista with SP1 and Windows Server 2008.
*/
VOID BlLoadImageNotifyRoutine(
	PUNICODE_STRING  FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO  ImageInfo
);

VOID ImageLogger(
	PUNICODE_STRING  FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO  ImageInfo
);

VOID SendLoadImageNotification(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,                // pid into which image is being mapped
	_In_ PIMAGE_INFO ImageInfo
);

////////////////////////////////////////////////
// Initialize / Destroy
////////////////////////////////////////////////

// TODO Check for OS Version. Ref: https://github.com/OSRDrivers/kmexts
NTSTATUS InitializeImageCallbacks() {
	NTSTATUS status;

	status = PsSetLoadImageNotifyRoutine(BlLoadImageNotifyRoutine);
	if (!NT_SUCCESS(status)) {
		LogError("Error, process notify registartion failed with code:%08x", status);
		DestroyImageCallbacks();
		return status;
	}
	g_PsSetLoadImageNotifyRoutineCreated = TRUE;

	LogTrace("Image callbacks initialization is completed");
	return STATUS_SUCCESS;
}

VOID DestroyImageCallbacks() {
	if (g_PsSetLoadImageNotifyRoutineCreated)
		PsRemoveLoadImageNotifyRoutine(BlLoadImageNotifyRoutine);

	LogTrace("Image callbacks deinitialization is completed");
}

////////////////////////////////////////////////
// Image Callbacks
////////////////////////////////////////////////

//
// The callback functions will call for action functions
// accordding to what action we want to preform (Log, StartInjection, SendEvents...)
//

VOID BlLoadImageNotifyRoutine(
	PUNICODE_STRING  FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO  ImageInfo
) {
	UNREFERENCED_PARAMETER(FullImageName);
	UNREFERENCED_PARAMETER(ProcessId);
	UNREFERENCED_PARAMETER(ImageInfo);

	//ImageLogger(FullImageName, ProcessId, ImageInfo);

	if (ProcessId == NULL)
		return;

	SendLoadImageNotification(
		FullImageName,
		ProcessId,
		ImageInfo
	);

	InjLoadImageNotifyRoutine(FullImageName, ProcessId, ImageInfo);

	return;
}

////////////////////////////////////////////////
// Action Rutines
////////////////////////////////////////////////

VOID ImageLogger(
	PUNICODE_STRING  FullImageName,
	HANDLE ProcessId,
	PIMAGE_INFO  ImageInfo
) {
	PIMAGE_INFO_EX imageInfoEx = NULL;
	PFILE_OBJECT   backingFileObject;

	// 
	// IMAGE_INFO_EX available on Vista, which will give us the
	// backing file object of the image section.
	// 
	if (ImageInfo->ExtendedInfoPresent) {
		imageInfoEx = CONTAINING_RECORD(ImageInfo, IMAGE_INFO_EX, ImageInfo);

		backingFileObject = imageInfoEx->FileObject;
	}
	else {
		backingFileObject = NULL;
	}


	LogTrace("%wZ being loaded into Process %u. Backing File Object %s (0x%p)",
		FullImageName,
		HandleToULong(ProcessId),
		backingFileObject != NULL ? "Available" : "Unavailable",
		backingFileObject);
}

VOID SendLoadImageNotification(
	_In_opt_ PUNICODE_STRING FullImageName,
	_In_ HANDLE ProcessId,                // pid into which image is being mapped
	_In_ PIMAGE_INFO ImageInfo
) {
	PBl_LoadImagePacket message = NULL;
	ULONG messageSize = sizeof(Bl_LoadImagePacket);
	ULONG imageNameSize = 0;

	if (FullImageName) {
		imageNameSize = FullImageName->Length;
		messageSize += imageNameSize;
	}

	message = (PBl_LoadImagePacket)ExAllocatePoolWithTag(NonPagedPool, messageSize, LOAD_IMAGE_TAG);
	if (!message) {
		LogWarning("Could not allocate memory for load image message");
		return;
	}

	// Header
	KeQuerySystemTimePrecise(&message->Header.time);
	message->Header.type = BlLoadImage;
	message->Header.size = messageSize;

	// Data
	message->ProcessId = HandleToULong(ProcessId);
	message->ImageBaseAddress = ImageInfo->ImageBase;

	message->ImageNameLength = 0;
	message->ImageNameOffset = sizeof(Bl_LoadImagePacket);
	if (imageNameSize > 0) {
		memcpy_s(
			(UCHAR*)message + message->ImageNameOffset,
			imageNameSize,
			FullImageName->Buffer,
			imageNameSize
		);
		message->ImageNameLength = imageNameSize / sizeof(WCHAR);
	}

	BlSendMessage(message, message->Header.size);

	ExFreePoolWithTag(message, LOAD_IMAGE_TAG);
}