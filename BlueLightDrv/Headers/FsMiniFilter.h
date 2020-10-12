#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#define PROCESS_CREATE_TAG  'cpLB' // BLpc - BlueLight Process Create
#define LOAD_IMAGE_TAG      'ilLB' // BLli - BlueLight Load Image

////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////

typedef struct _BL_GLOBAL_FILTER_DATA {
    //
    //  The global FLT_FILTER pointer. Many API needs this, such as 
    //  FltAllocateContext(...)
    //

    PFLT_FILTER Filter;

    //
    //  Server-side communicate ports.
    //

    PFLT_PORT BlServerPort;

    //
    //  The scan client ports.
    //

    PFLT_PORT BlClientPort;

    //
    //  A flag that indicating that the filter is being unloaded.
    //    

    BOOLEAN  Unloading;

} BL_GLOBAL_FILTER_DATA, * PBL_GLOBAL_FILTER_DATA;

////////////////////////////////////////////////
// Globals 
////////////////////////////////////////////////

BL_GLOBAL_FILTER_DATA Globals;

////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////

NTSTATUS InitializeFsMiniFilter(PDRIVER_OBJECT DriverObject);
NTSTATUS DestroyFsMiniFilter();
