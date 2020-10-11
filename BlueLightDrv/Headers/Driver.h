#pragma once

#include <ntddk.h>

VOID SetDriverState(BOOLEAN enabled);
BOOLEAN IsDriverActive();
