#include "Movement.h"
#include <Windows.h>
#include <iostream>
#include <cstdio>
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
namespace Movement {
    bool BunnyhopEnabled = false;

    void BunnyHop() {

        if (!BunnyhopEnabled)
            return;
        //printf(" Codigo \n");
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;
        try {
            uintptr_t localPlayer = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) {
            //printf(" N i g g e r ");
                return;
            }
            //printf(" :D ");

            int flags = Memory::Read<int>(localPlayer + offsets::m_fFlags);

            if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
               //printf(" Space Pressed ");
                if (flags & (1 << 0)) {
                    Memory::Write<int>(Memory::clientDll + offsets::dwForceJump, 6);
                }
            }
        }
        catch (...) { printf("[Bunnyhop] exception\n"); }
    }
}
