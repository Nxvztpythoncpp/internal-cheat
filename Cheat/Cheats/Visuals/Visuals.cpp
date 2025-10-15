// Visuals.cpp
#include "Visuals.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <cstddef>

namespace Visuals {
    // Define the variables declared in the header
    bool Visuals::glowEnabled = false;
    bool Visuals::glowThroughWalls = false;
    float Visuals::glowColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };      // default glow color: red (RGBA)

    void Glow() {
        if (!glowEnabled)
            return;
        // Ensure the client module base address is initialized
        if (Memory::clientDll == 0)
            return;

        // Get pointer to the glow object manager
        std::uintptr_t glowManagerPtr = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwGlowObjectManager);
        if (!glowManagerPtr)
            return;  // glow manager not found, skip

        // Get the local player for team checks
        std::uintptr_t localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
        int localTeam = localPlayer ? Memory::Read<int>(localPlayer + offsets::m_iTeamNum) : -1;

        // Iterate through player entities (1 to 64 max)
        for (int i = 1; i <= 64; ++i) {
            std::uintptr_t entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
            if (!entity) continue;

            // Skip dormant entities
            bool dormant = Memory::Read<bool>(entity + offsets::m_bDormant);
            if (dormant) continue;
            // Only glow enemies (skip if same team as local or no local player)
            int entityTeam = Memory::Read<int>(entity + offsets::m_iTeamNum);
            if (entityTeam == localTeam || entityTeam == 0) continue;
            // Skip dead entities
            int health = Memory::Read<int>(entity + offsets::m_iHealth);
            if (health < 1) continue;

            // Get this entity's glow index
            int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
            if (glowIndex < 0) continue;

            constexpr std::size_t GLOW_OBJECT_SIZE = 0x38;       // Each glow object is 0x38 bytes
            constexpr std::uintptr_t GLOW_COLOR_OFFSET = 0x8;    // Offset to glow color Vector (R at +0x8)
            constexpr std::uintptr_t GLOW_ALPHA_OFFSET = 0x14;   // Offset to glow alpha value
            constexpr std::uintptr_t GLOW_OCCLUDED_OFFSET = 0x28;    // bool m_bRenderWhenOccluded
            constexpr std::uintptr_t GLOW_UNOCCLUDED_OFFSET = 0x29;  // bool m_bRenderWhenUnoccluded
            constexpr std::uintptr_t GLOW_FULLBLOOM_OFFSET = 0x2A;   // bool m_bFullBloomRender
            constexpr std::uintptr_t GLOW_STYLE_OFFSET = 0x30;   // int m_nRenderStyle
            constexpr std::uintptr_t GLOW_ALPHA_MAX_OFFSET = 0x20;

            // Compute address of this entity's GlowObjectDefinition in the glow array
            std::uintptr_t glowObject = glowManagerPtr + glowIndex * GLOW_OBJECT_SIZE;
            // Write glow color (RGBA) from our config color
            Memory::Write<float>(glowObject + GLOW_COLOR_OFFSET + 0x0, glowColor[0]);  // R
            Memory::Write<float>(glowObject + GLOW_COLOR_OFFSET + 0x4, glowColor[1]);  // G
            Memory::Write<float>(glowObject + GLOW_COLOR_OFFSET + 0x8, glowColor[2]);  // B
            Memory::Write<float>(glowObject + GLOW_ALPHA_OFFSET, glowColor[3]);  // Alpha (transparency)
            // Set render style and behavior flags
            Memory::Write<bool>(glowObject + GLOW_OCCLUDED_OFFSET, true);                     // Render when occluded (behind walls)
            Memory::Write<bool>(glowObject + GLOW_UNOCCLUDED_OFFSET, glowThroughWalls ? true : true);  // Always render when visible; we keep this true
            Memory::Write<bool>(glowObject + GLOW_FULLBLOOM_OFFSET, false);                    // No full-bloom glow
            Memory::Write<int>(glowObject + GLOW_STYLE_OFFSET, 0);                        // Glow style 0 (default outline)
            Memory::Write<float>(glowObject + GLOW_ALPHA_MAX_OFFSET, 1.0f);                     // Max glow alpha
        }
    }
}
