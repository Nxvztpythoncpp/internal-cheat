#include "Radar.h"     
#include "../Config/config_vars.h"  // g_cfg
#include <cstdint>
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"

// idk done it in like 5 minutes so dont judge me

void Radar::Run() {
    if (!g_cfg::misc::radar)
        return;  

    uintptr_t localPlayer = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
    if (!localPlayer) return;
    int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
    int localIndex = Memory::Read<int>(localPlayer + 0x64);

    for (int i = 1; i < 32; i++) {
        uintptr_t entity = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
        if (!entity) continue;
        if (Memory::Read<bool>(entity + offsets::m_bDormant)) continue;
        int health = Memory::Read<int>(entity + offsets::m_iHealth);
        if (health <= 0 || health > 100) continue;

        // Team check: skip 
        int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
        if (team == localTeam || team == 0) continue;

        uint32_t mask = Memory::Read<uint32_t>(entity + offsets::m_bSpottedByMask);
        mask |= (1u << (localIndex - 1));
        Memory::Write<uint32_t>(entity + offsets::m_bSpottedByMask, mask);
    }
}
