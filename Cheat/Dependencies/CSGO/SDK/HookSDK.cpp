// Hooks.cpp
#include <Windows.h>
#include <cstdio>
#include "../../MinHook/MinHook.h"
#include "HookSDK.h"
#include "../../Hooks/Hooks.h"
#include "../../../Cheats/Rage/Aimbot.h"
#include "../../../Cheats/SkinChanger/SkinChanger.h"
#include "../../../Memory/Memory.h"
#include "../../../Memory/Offsets.h"
#include "../../CacheSystem/CacheSystem.h"

static void* g_pClientMode = nullptr;
IBaseClientDLL* g_pClient = nullptr;

// ---------------- CreateMove ----------------
typedef void(__thiscall* CreateMoveFn)(void*, int, float, bool);
CreateMoveFn oCreateMove = nullptr;

void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
    if (!oCreateMove) return;
    oCreateMove(g_pClientMode, sequence_number, input_sample_frametime, active);

    if (Hooks::menu_open)
    {
        // Optional: block movement
    }

    if (Aimbot::enabled)
    {
        if (Aimbot::silent)
            Aimbot::OnCreateMove();
        else
            Aimbot::Run();
    }
}

// ---------------- GetUserCmd (optional) ----------------
typedef void* (__thiscall* GetUserCmdFn)(void*, int);
GetUserCmdFn oGetUserCmd = nullptr;

void* __stdcall hkGetUserCmd(void* thisptr, int sequence_number)
{
    void* cmd = oGetUserCmd(thisptr, sequence_number);
    if (cmd && Hooks::menu_open)
    {
        CUserCmd* userCmd = (CUserCmd*)cmd;
        userCmd->buttons = 0;
        userCmd->forwardmove = 0.0f;
        userCmd->sidemove = 0.0f;
        userCmd->upmove = 0.0f;
        userCmd->mousedx = 0;
        userCmd->mousedy = 0;
    }
    return cmd;
}

// ---------------- FrameStageNotify ----------------
FrameStageNotify_t oFrameStageNotify = nullptr;

void __fastcall hkFrameStageNotify(void* thisptr, void* edx, ClientFrameStage_t stage)
{
    printf("[HOOK] hkFrameStageNotify called | stage=%d | this=0x%p\n", (int)stage, thisptr);

    // Run SkinChanger only if enabled and on the right stage
    if (SkinChanger::enabled && stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
    {
        printf("[SKIN] FRAME_NET_UPDATE_POSTDATAUPDATE_START hit - calling ApplySkins\n");
        SkinChanger::ApplySkins();
        // ApplySkins disables SkinChanger::enabled when done
        printf("[SKIN] ApplySkins returned\n");
    }

    // Call original
    if (oFrameStageNotify)
        oFrameStageNotify(thisptr, stage);
}

// ---------------- Hook Setup ----------------
void SetupSDKHooks()
{
    if (MH_Initialize() != MH_OK)
        printf("[HOOK] MinHook failed to initialize\n");

    g_pClientMode = GetInterface("client.dll", "VClient018");
    if (!g_pClientMode)
        g_pClientMode = GetInterface("client.dll", "VClient017");

    if (g_pClientMode)
    {
        oCreateMove = GetVFunc<CreateMoveFn>(g_pClientMode, 21);
        if (oCreateMove && MH_CreateHook(oCreateMove, &hkCreateMove, reinterpret_cast<LPVOID*>(&oCreateMove)) == MH_OK)
        {
            MH_EnableHook(oCreateMove);
            printf("[HOOK] CreateMove hook enabled\n");
        }
    }

    g_pClient = (IBaseClientDLL*)GetInterface("client.dll", "VClient018");
    if (!g_pClient)
        g_pClient = (IBaseClientDLL*)GetInterface("client.dll", "VClient017");

    if (g_pClient)
    {
        void* pFn = GetVFunc<void*>(g_pClient, 37);
        if (pFn)
        {
            oFrameStageNotify = reinterpret_cast<FrameStageNotify_t>(pFn);
            if (MH_CreateHook(pFn, &hkFrameStageNotify, reinterpret_cast<LPVOID*>(&oFrameStageNotify)) == MH_OK)
            {
                MH_EnableHook(pFn);
                printf("[HOOK] FrameStageNotify hook enabled\n");
            }
            else
            {
                printf("[HOOK] Failed to create FrameStageNotify hook\n");
            }
        }
        else
        {
            printf("[HOOK] Failed to find vfunc[37] for FrameStageNotify\n");
        }
    }
    else
    {
        printf("[HOOK] Failed to get IBaseClientDLL interface\n");
    }
}
