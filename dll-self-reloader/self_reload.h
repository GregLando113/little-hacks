#pragma once
#include <Windows.h>
 
// by KAOS
 
// Give it module handle that will be reloaded, then finish up your shit and unload yourself. 
// It will reload the module immediately after.
HANDLE SetupModuleReload(HMODULE module);