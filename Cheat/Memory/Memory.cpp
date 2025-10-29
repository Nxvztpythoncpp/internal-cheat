#include "Memory.h"
#include <windows.h>
#include <TlHelp32.h>
#include <string>

namespace Memory
{
    uintptr_t clientDll = 0;
    uintptr_t engineDll = 0;

    static uintptr_t GetModuleBase(const char* moduleName)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            MessageBoxA(nullptr, "Failed to create module snapshot", "Memory Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        MODULEENTRY32 moduleEntry{ sizeof(MODULEENTRY32) };
        if (!Module32First(hSnapshot, &moduleEntry))
        {
            CloseHandle(hSnapshot);
            MessageBoxA(nullptr, "No modules found in snapshot", "Memory Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        do
        {
            char modName[MAX_MODULE_NAME32 + 1];
            size_t converted = 0;
            wcstombs_s(&converted, modName, moduleEntry.szModule, MAX_MODULE_NAME32 + 1);

            if (_stricmp(modName, moduleName) == 0)
            {
                CloseHandle(hSnapshot);
                return reinterpret_cast<uintptr_t>(moduleEntry.modBaseAddr);
            }
        } while (Module32Next(hSnapshot, &moduleEntry));

        CloseHandle(hSnapshot);
        std::string err = "Module not found: " + std::string(moduleName);
        MessageBoxA(nullptr, err.c_str(), "Memory Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    bool Initialize()
    {
        // Try multiple times to wait for modules to load
        for (int i = 0; i < 10; ++i)
        {
            clientDll = GetModuleBase("client.dll");
            if (!clientDll)
                clientDll = GetModuleBase("client_panorama.dll"); // older builds fallback

            engineDll = GetModuleBase("engine.dll");

            if (clientDll && engineDll)
            {
                char buf[128];
                sprintf_s(buf, "client.dll base: 0x%llX\nengine.dll base: 0x%llX",
                    static_cast<unsigned long long>(clientDll),
                    static_cast<unsigned long long>(engineDll));
                OutputDebugStringA(buf);
                return true;
            }

            Sleep(1000);
        }

        MessageBoxA(nullptr, "Failed to find client/engine modules (are you injected into CS:GO?)",
            "Memory Error", MB_OK | MB_ICONERROR);
        return false;
    }
}
