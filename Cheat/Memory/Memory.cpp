// Memory.cpp
#include "Memory.h"
#include <windows.h>
#include <TlHelp32.h>
#include <string>

namespace Memory {
    DWORD clientDll = 0;
    DWORD engineDll = 0;

    DWORD GetModuleBase(const char* moduleName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            MessageBoxA(NULL, "Failed to create module snapshot", "Memory Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        MODULEENTRY32 moduleEntry{ sizeof(MODULEENTRY32) };
        if (!Module32First(hSnapshot, &moduleEntry)) {
            CloseHandle(hSnapshot);
            MessageBoxA(NULL, "No modules found in snapshot", "Memory Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        do {
            wchar_t wModuleName[MAX_MODULE_NAME32 + 1];
            size_t converted = 0;
            mbstowcs_s(&converted, wModuleName, moduleName, MAX_MODULE_NAME32 + 1);

            // Case-insensitive comparison
            if (_wcsicmp(moduleEntry.szModule, wModuleName) == 0) {
                CloseHandle(hSnapshot);
                return reinterpret_cast<DWORD>(moduleEntry.modBaseAddr);
            }
        } while (Module32Next(hSnapshot, &moduleEntry));

        CloseHandle(hSnapshot);
        std::string errorMsg = "Module not found: " + std::string(moduleName);
        MessageBoxA(NULL, errorMsg.c_str(), "Memory Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    bool Initialize() {
        // Wait for CS:GO to load modules (max 10 seconds)
        const int maxAttempts = 10;
        int attempt = 0;
        while (attempt < maxAttempts) {
            clientDll = GetModuleBase("client.dll");
            if (clientDll == 0) {
                // Fallback: Try client_panorama.dll
                clientDll = GetModuleBase("client_panorama.dll");
            }
            engineDll = GetModuleBase("engine.dll");

            if (clientDll != 0) {
                // clientDll is enough for Visuals.cpp
                return true;
            }

            Sleep(1000); // Wait 1 second before retrying
            attempt++;
        }

        MessageBoxA(NULL, "Failed to initialize after retries. Ensure CS:GO is running.", "Memory Error", MB_OK | MB_ICONERROR);
        return false;
    }
}