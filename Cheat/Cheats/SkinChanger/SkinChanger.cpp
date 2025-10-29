#include "SkinChanger.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <Windows.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <cstdio>

namespace SkinChanger
{
    std::unordered_map<int, WeaponSkin> skinConfig;
    static std::unordered_map<int, int> lastAppliedPaint;
    static std::unordered_map<uintptr_t, bool> appliedWeapons;
    constexpr size_t MAX_CUSTOM_NAME = 32;

    const char* GetWeaponName(int defIndex)
    {
        for (size_t i = 0; i < weaponCount; ++i)
            if (weaponIDs[i] == defIndex)
                return weaponNames[i];
        return "Unknown";
    }

    static void WriteCustomName(uintptr_t weapon, const std::string& name)
    {
        if (!weapon) return;

        uintptr_t addr = weapon + offsets::m_szCustomName;
        char buffer[32] = { 0 };
        size_t len = (name.size() < sizeof(buffer) - 1) ? name.size() : (sizeof(buffer) - 1);

        memcpy(buffer, name.c_str(), len);
        memcpy(reinterpret_cast<void*>(addr), buffer, sizeof(buffer));
    }


    static void ApplySkinToWeapon(uintptr_t weapon, const WeaponSkin& skin)
    {
        if (!weapon) return;

        short defIndex = *reinterpret_cast<short*>(weapon + offsets::m_iItemDefinitionIndex);
        if (defIndex <= 0) return;

        int paintKit = std::clamp(skin.paintKit, 0, 200000);
        float wear = std::clamp(skin.wear, 0.0f, 1.0f);

        *reinterpret_cast<int*>(weapon + offsets::m_nFallbackPaintKit) = paintKit;
        *reinterpret_cast<float*>(weapon + offsets::m_flFallbackWear) = wear;
        *reinterpret_cast<int*>(weapon + offsets::m_nFallbackSeed) = skin.seed;
        *reinterpret_cast<int*>(weapon + offsets::m_nFallbackStatTrak) = skin.statTrak ? skin.statTrakCount : -1;
        *reinterpret_cast<int*>(weapon + offsets::m_iItemIDHigh) = -1;
        *reinterpret_cast<int*>(weapon + offsets::m_iEntityQuality) = 3;

        WriteCustomName(weapon, skin.customName);
    }


    static void TryForceFullUpdate()
    {
        static bool updateSent = false;
        if (updateSent) return;

        uintptr_t clientState = Memory::Read<uintptr_t>(Memory::engineDll + offsets::dwClientState);
        if (!clientState) return;

        Memory::Write<int>(clientState + offsets::clientstate_delta_ticks, -1);
        updateSent = true;
        printf("[SkinChanger] ForceFullUpdate sent\n");
    }

    void Run()
    {
        if (!enabled) {
            appliedWeapons.clear();
            lastAppliedPaint.clear();
            return;
        }

        if (!Memory::clientDll || !Memory::engineDll) return;

        uintptr_t localPlayer = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return;

        bool skinChanged = false;

        for (int i = 0; i < 8; ++i)
        {
            uint32_t handle = Memory::Read<uint32_t>(localPlayer + offsets::m_hMyWeapons + i * 4);
            if (!handle) continue;

            uint32_t idx = handle & 0xFFF;
            if (idx == 0 || idx > 2048) continue;

            uintptr_t weapon = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwEntityList + (idx - 1) * 0x10);
            if (!weapon) continue;

            short defIndex = Memory::Read<short>(weapon + offsets::m_iItemDefinitionIndex);
            if (defIndex <= 0) continue;

            auto it = skinConfig.find(defIndex);
            if (it == skinConfig.end()) continue;

            int currentPaint = Memory::Read<int>(weapon + offsets::m_nFallbackPaintKit);
            int targetPaint = it->second.paintKit;

            // Aplica apenas se ainda não aplicado
            if (!appliedWeapons[weapon] || currentPaint != targetPaint || lastAppliedPaint[defIndex] != targetPaint)
            {
                ApplySkinToWeapon(weapon, it->second);
                appliedWeapons[weapon] = true;
                skinChanged = true;
            }
        }

        if (skinChanged)
            TryForceFullUpdate();
    }

    void SaveSkins() {}
    void LoadSkins() {}
}
