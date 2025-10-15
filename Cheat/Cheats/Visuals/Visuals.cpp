// Visuals.cpp
#include "Visuals.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"

namespace Visuals {
    // Define the variables declared in the header
    bool glowEnabled = false;
    bool glowThroughWalls = true;                      // true = glow visible through walls by default
    float glowColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };      // default glow color: red (RGBA)

    void Glow() {
        if (!glowEnabled) return;  // Only run if glow ESP is enabled

        // Get local player base address
        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return;

        // Get pointer to the Glow Object Manager
        uintptr_t glowManager = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwGlowObjectManager);
        if (!glowManager) return;

        // Read local player's team (to differentiate enemies)
        int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);

        // Loop through player entities (1-32)
        for (int i = 1; i <= 32; ++i) {
            uintptr_t entity = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwEntityList + i * 0x10);
            if (!entity) continue;

            // Basic validity checks (health and dormancy)
            int health = Memory::Read<int>(entity + offsets::m_iHealth);
            if (health <= 0) continue;  // dead or not a player
            bool dormant = Memory::Read<bool>(entity + offsets::m_bDormant);
            if (dormant) continue;      // skip dormant entities

            // Team check – skip if entity is on the same team (only glow enemies)
            int entityTeam = Memory::Read<int>(entity + offsets::m_iTeamNum);
            if (entityTeam == localTeam) continue;

            // Visibility check – if glowThroughWalls is false, only glow if the enemy is visible
            if (!glowThroughWalls) {
                uint32_t spottedMask = Memory::Read<uint32_t>(entity + offsets::m_bSpottedByMask);
                int localIndex = Memory::Read<int>(localPlayer + 0x64);  // local player index
                bool visible = (spottedMask & (1 << (localIndex - 1))) != 0;
                if (!visible) continue;  // skip this entity if not visible and we only want visible glow
            }

            // Get the glow index for this entity
            int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
            if (glowIndex < 0) continue;  // invalid glow index

            // Write the glow color into the glow array for this entity
            // Each glow object is 0x38 bytes in size; color RGBA starts at offset 0x4 in the glow object structure:contentReference[oaicite:0]{index=0}.
            Memory::Write<float>(glowManager + glowIndex * 0x38 + 0x4, glowColor[0]);  // R
            Memory::Write<float>(glowManager + glowIndex * 0x38 + 0x8, glowColor[1]);  // G
            Memory::Write<float>(glowManager + glowIndex * 0x38 + 0xC, glowColor[2]);  // B
            Memory::Write<float>(glowManager + glowIndex * 0x38 + 0x10, glowColor[3]);  // A

            // Set glow render flags – whether to glow when occluded and unoccluded
            // 0x24 and 0x25 are the offsets for m_bRenderWhenOccluded and m_bRenderWhenUnoccluded:contentReference[oaicite:1]{index=1}.
            bool occluded = true;     // render glow when occluded (behind walls)
            bool unoccluded = true;   // render glow when unoccluded (visible normally)
            if (!glowThroughWalls) {
                occluded = false;   // if we only want visible glow, disable glow through walls
                unoccluded = true;    // still glow when the entity is visible
            }
            Memory::Write<bool>(glowManager + glowIndex * 0x38 + 0x24, occluded);
            Memory::Write<bool>(glowManager + glowIndex * 0x38 + 0x25, unoccluded);
        }
    }
}
