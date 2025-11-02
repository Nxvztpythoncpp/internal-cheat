#pragma once
#include <Windows.h>
#include "../../MinHook/MinHook.h"

// Forward declarations
class IBaseClientDLL;
class IClientMode;
class CUserCmd;
class C_BaseEntity;
class C_BasePlayer;
class C_BaseCombatWeapon;
class IClientEntityList;

extern IClientEntityList* g_pClientEntityList;

// Valve FrameStage enum
enum ClientFrameStage_t
{
    FRAME_UNDEFINED = -1,
    FRAME_START = 0,
    FRAME_NET_UPDATE_START = 1,
    FRAME_NET_UPDATE_POSTDATAUPDATE_START = 2,
    FRAME_NET_UPDATE_POSTDATAUPDATE_END = 3,
    FRAME_NET_UPDATE_END = 4,
    FRAME_RENDER_START = 5,
    FRAME_RENDER_END = 6
};

// Correct typedef for IBaseClientDLL::FrameStageNotify
typedef void(__thiscall* FrameStageNotify_t)(void* thisptr, ClientFrameStage_t stage);

// Typedef for CreateInterface
typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

// Interface fetch helper
inline void* GetInterface(const char* moduleName, const char* interfaceName)
{
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (!hModule) return nullptr;

    CreateInterfaceFn createInterface = (CreateInterfaceFn)GetProcAddress(hModule, "CreateInterface");
    if (!createInterface) return nullptr;

    int returnCode = 0;
    return createInterface(interfaceName, &returnCode);
}

// Generic vfunc fetcher
template<typename T>
T GetVFunc(void* instance, int index)
{
    uintptr_t* vtable = *(uintptr_t**)instance;
    return (T)vtable[index];
}

// Globals
extern void* g_pClientMode;
extern IBaseClientDLL* g_pClient;

// Extern hook originals
extern FrameStageNotify_t oFrameStageNotify;

// Forward declarations
extern void ApplySkins();
void SetupSDKHooks();
