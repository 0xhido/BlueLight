#pragma once

/* 
* Contains configuration objects fetched for the registry
*/

NTSTATUS InitializeConfigs(PUNICODE_STRING RegistryPath);
NTSTATUS DestroyConfigs();