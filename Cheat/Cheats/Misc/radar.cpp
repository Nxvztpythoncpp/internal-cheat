#include "Radar.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include "../Config/config_vars.h"
#include "../../Gui/Imgui/imgui.h"

namespace RadarHack {

    // Runs the radar hack: marks all (valid) entities as spotted on the radar
    void Run() {
        // Check if radar hack is enabled in config
        if (!g_cfg::misc::radar)
            return;
        // Ensure memory (client module) is initialized
        if (Memory::clientDll == 0 && !Memory::Initialize())
            return;

        // Get local player and team
        auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer)
            return;
        int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);

        // Loop through possible player entities
        for (int i = 1; i < 64; i++) {
            // Read the entity pointer from the entity list
            std::uintptr_t entity = Memory::Read<std::uintptr_t>(
                Memory::clientDll + offsets::dwEntityList + i * 0x10);
            if (!entity)
                continue;
            // Skip if the entity is dormant (not active)
            bool isDormant = Memory::Read<bool>(entity + offsets::m_bDormant);
            if (isDormant)
                continue;
            // Skip if the entity is dead
            int health = Memory::Read<int>(entity + offsets::m_iHealth);
            if (health <= 0)
                continue;
            // Skip teammates if team-check is enabled
            int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
            if (g_cfg::misc::radar_teamCheck && team == localTeam)
                continue;
            // Finally, write to m_bSpotted = true to force on radar
            Memory::Write<bool>(entity + offsets::m_bSpottedByMask, true);
        }
    }
} // namespace RadarHack
