#include "../../MinHook/MinHook.h"
#include "HookSDK.h"
#include "../../Hooks/Hooks.h"
#include "../../../Cheats/Rage/Aimbot.h"

#pragma message("Incluindo hook_sdk.h em HookSDK.cpp")

static void* g_pClientMode = nullptr;

typedef void(__thiscall* CreateMoveFn)(void*, int, float, bool);
CreateMoveFn oCreateMove = nullptr;

void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active) {
    if (!oCreateMove) return;

    // Call original first
    oCreateMove(g_pClientMode, sequence_number, input_sample_frametime, active);

    if (Hooks::menu_open) {
        // Block game input when menu is open
    }

    // Run aimbot
    if (Aimbot::enabled) {
        if (Aimbot::silent)
            Aimbot::OnCreateMove();
        else
            Aimbot::Run();
    }
}

// Alternative approach - hook the input system directly
typedef void* (__thiscall* GetUserCmdFn)(void*, int);
GetUserCmdFn oGetUserCmd = nullptr;

void* __stdcall hkGetUserCmd(void* thisptr, int sequence_number) {
    void* cmd = oGetUserCmd(thisptr, sequence_number);

    if (cmd && Hooks::menu_open) {
        CUserCmd* userCmd = (CUserCmd*)cmd;
        // Block all movement
        userCmd->buttons = 0;
        userCmd->forwardmove = 0.0f;
        userCmd->sidemove = 0.0f;
        userCmd->upmove = 0.0f;
        userCmd->mousedx = 0;
        userCmd->mousedy = 0;
    }

    return cmd;
}

void SetupSDKHooks() {
    // Get client mode interface
    g_pClientMode = GetInterface("client.dll", "VClient018");
    if (!g_pClientMode) {
        g_pClientMode = GetInterface("client.dll", "VClient017");
    }

    if (g_pClientMode) {
        // CreateMove is usually at index 21 or 22
        oCreateMove = GetVFunc<CreateMoveFn>(g_pClientMode, 21);
        if (oCreateMove) {
            if (MH_CreateHook(oCreateMove, &hkCreateMove, reinterpret_cast<LPVOID*>(&oCreateMove)) == MH_OK) {
                MH_EnableHook(oCreateMove);
            }
        }
    }
}