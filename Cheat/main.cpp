#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <cstdio>
#include <atomic>
#include "Dependencies/Hooks/Hooks.h"
#include "Cheats/Misc/UninjectHook.h"
#include <iostream>

HMODULE g_hModule = nullptr;

BOOL APIENTRY DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        g_hModule = hMod;
        DisableThreadLibraryCalls(hMod);
        Unhook::SetModuleHandle(hMod);
        //Unhook::ResetGlobalState();

        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Initialize, hMod, 0, nullptr);
        std::cerr << "DLL injected successfully" << std::endl;
        break;

    case DLL_PROCESS_DETACH:
        //Unhook::RequestUnload();
        //Unhook::StartSafeCleanupThread();
        //std::cerr << "DLL detach requested" << std::endl;
        break;
    }
    return TRUE;
}

#ifdef _WIN32
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nShowCmd;
    return 0;
}
#endif