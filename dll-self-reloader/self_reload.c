#include "self_reload.h"
 
typedef HMODULE (WINAPI *LoadLibraryW_t)(
  _In_ LPCWSTR lpFileName
);
 
typedef VOID (WINAPI *Sleep_t)(
  _In_ DWORD dwMilliseconds
);
 
typedef HMODULE (WINAPI *GetModuleHandleW_t)(
  _In_opt_ LPCWSTR lpModuleName
);
 
typedef struct _RELOAD_INFO {
    HANDLE 			module;
    WCHAR 			module_path[MAX_PATH];
    Sleep_t 			sleep;
    LoadLibraryW_t 		loadlib;
    GetModuleHandleW_t 	getmodulehandle;
} RELOAD_INFO;
 
static DWORD WINAPI stub_ReloadModule(RELOAD_INFO* info) {
	while (info->getmodulehandle(info->module_path)){
		info->sleep(100);
	}
	return (DWORD)info->loadlib(info->module_path);
}
static void stubend_ReloadModule(void) {}
 
HANDLE SetupModuleReload(HMODULE module){
    size_t size_req = (uintptr_t)stubend_ReloadModule - (uintptr_t)stub_ReloadModule + sizeof(RELOAD_INFO);
    LPVOID buffer_base = VirtualAlloc(nullptr, size_req, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
 
    if(!buffer_base)
        return false;
 
    RELOAD_INFO* info = (RELOAD_INFO*)buffer_base;
    LPTHREAD_START_ROUTINE code = (LPTHREAD_START_ROUTINE)((BYTE*)buffer_base + sizeof(RELOAD_INFO));
 
    HMODULE k32 = GetModuleHandleA("kernel32.dll");
 
    info->module = module;
    info->loadlib = (LoadLibraryW_t)GetProcAddress(k32,"LoadLibraryW");
	info->sleep = (Sleep_t)GetProcAddress(k32,"Sleep");
	info->getmodulehandle = (GetModuleHandleW_t)GetProcAddress(k32,"GetModuleHandleW");
 
    if(!GetModuleFileNameW(module, info->module_path, MAX_PATH))
        return false;
 
    memcpy(code,(void*)stub_ReloadModule,(uintptr_t)stubend_ReloadModule - (uintptr_t)stub_ReloadModule);
 
	HANDLE thread = CreateThread(0,0,code,info,0,0);
	
    return thread;
}