#include <ntddk.h>

#include "Callbacks/CbImage.h"

#include "Helper.h"

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