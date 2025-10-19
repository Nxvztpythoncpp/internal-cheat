

#include <Windows.h>
#include "Dependencies/Hooks/Hooks.h"
#include "Dependencies/Kiero/kiero.h"
#include <cstdlib>
#include <stdio.h>



BOOL APIENTRY DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hMod);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Initialize, hMod, 0, nullptr);

        //CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Console, hMod, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        kiero::shutdown();
        break;
    }
    return TRUE;
}

int main()
{
    
    // put it here. Keeping it minimal so it doesn't interfere with your cheat.
    return 0;
}

#ifdef _WIN32
// Provide WinMain for GUI subsystem builds; forward to main() so both linkers work.
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nShowCmd;
    return main();
}
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif