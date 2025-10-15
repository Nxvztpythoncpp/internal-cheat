#include "Memory.h"
#include <windows.h>
#include <TlHelp32.h>
#include <string>

namespace Memory {
    DWORD clientDll = 0;
    DWORD engineDll = 0;

    DWORD GetModuleBase(const char* moduleName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
        if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(hSnapshot, &moduleEntry)) {
            do {                
                wchar_t wModuleName[MAX_MODULE_NAME32 + 1];
                size_t converted = 0;
                mbstowcs_s(&converted, wModuleName, moduleName, MAX_MODULE_NAME32 + 1);

                if (wcscmp(moduleEntry.szModule, wModuleName) == 0) {
                    CloseHandle(hSnapshot);
                    return (DWORD)moduleEntry.modBaseAddr;
                }
            } while (Module32Next(hSnapshot, &moduleEntry));
        }

        CloseHandle(hSnapshot);
        return 0;
    }

    bool Initialize() {
        clientDll = GetModuleBase("client.dll");
        engineDll = GetModuleBase("engine.dll");

        return (clientDll != 0 && engineDll != 0);
    }
}