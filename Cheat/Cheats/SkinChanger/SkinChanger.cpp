// SkinChanger.cpp
#include "SkinChanger.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <Windows.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdio>
#include "../../Gui/Menu/Skins/skins_db.h" // assume este header fornece skins_db
#include "../../Dependencies/CacheSystem/CacheSystem.h"

namespace SkinChanger
{
    std::unordered_map<int, WeaponSkin> skinConfig;
    constexpr size_t MAX_CUSTOM_NAME = 32;

    void WriteCustomName(uintptr_t weapon, const std::string& name)
    {
        if (!weapon) return;
        char buffer[MAX_CUSTOM_NAME]{};
        strncpy_s(buffer, name.c_str(), sizeof(buffer) - 1);
        Memory::WriteArray(weapon + offsets::m_szCustomName, buffer, sizeof(buffer));
    }

    void ApplySkinToWeapon(uintptr_t weapon, const WeaponSkin& skin)
    {
        if (!weapon) return;

        using namespace Memory;
        using namespace offsets;

        // Write iItemIDHigh first to ensure paint kit updates properly on many builds
        Write<int>(weapon + m_iItemIDHigh, -1);

        // Then write fallback values
        Write<int>(weapon + m_nFallbackPaintKit, std::clamp(skin.paintKit, 0, 200000));
        Write<float>(weapon + m_flFallbackWear, std::clamp(skin.wear, 0.0f, 1.0f));
        Write<int>(weapon + m_nFallbackSeed, skin.seed);
        Write<int>(weapon + m_nFallbackStatTrak, skin.statTrak ? skin.statTrakCount : -1);
        Write<int>(weapon + m_iEntityQuality, 3);
        WriteCustomName(weapon, skin.customName);

        printf("[SKIN] ApplySkinToWeapon: paint=%d wear=%.6f seed=%d statTrak=%d weapon=0x%p\n",
            skin.paintKit, skin.wear, skin.seed, skin.statTrak ? skin.statTrakCount : -1, (void*)weapon);
    }

    void ResetWeaponSkin(uintptr_t weapon)
    {
        if (!weapon) return;
        using namespace Memory;
        using namespace offsets;
        Write<int>(weapon + m_nFallbackPaintKit, 0);
        Write<int>(weapon + m_nFallbackSeed, 0);
        Write<float>(weapon + m_flFallbackWear, 0.0f);
        Write<int>(weapon + m_nFallbackStatTrak, -1);
        Write<int>(weapon + m_iItemIDHigh, 0);
        // Clear custom name
        for (size_t i = 0; i < MAX_CUSTOM_NAME; ++i)
            Write<char>(weapon + m_szCustomName + i, '\0');

        printf("[SKIN] ResetWeaponSkin: reset weapon 0x%p\n", (void*)weapon);
    }

    void ApplySkins()
    {
        // header provides inline bool enabled; respect it
        if (!enabled) return;

        using namespace Memory;
        using namespace offsets;

        uintptr_t localPlayer = g_Cache.localPlayer;
        if (!localPlayer)
            localPlayer = Read<uintptr_t>(clientDll + dwLocalPlayer);

        if (!localPlayer)
        {
            printf("[SKIN] ApplySkins: localPlayer NULL - abort\n");
            enabled = false;
            return;
        }

        char buf[256];
        int appliedCount = 0;
        int weaponsFound = 0;

        for (int i = 0; i < 8; ++i)
        {
            uint32_t handle = Read<uint32_t>(localPlayer + m_hMyWeapons + i * 4);
            if (!handle) continue;
            uint32_t idx = handle & 0xFFF;
            if (idx == 0 || idx > 2048) continue;

            uintptr_t weapon = Read<uintptr_t>(clientDll + dwEntityList + (idx - 1) * 0x10);
            if (!weapon) continue;

            short defIndex = Read<short>(weapon + m_iItemDefinitionIndex);
            if (defIndex <= 0) continue;

            weaponsFound++;

            auto it = skinConfig.find(defIndex);
            if (it != skinConfig.end())
            {
                ApplySkinToWeapon(weapon, it->second);
                appliedCount++;
                sprintf_s(buf, "[SKIN] Applied def %d paint %d to weapon 0x%p", defIndex, it->second.paintKit, (void*)weapon);
                printf("%s\n", buf);
            }
            else
            {
                // optional debug for weapons without config
                // printf("[SKIN] No skin configured for defIndex %d (weapon=0x%p)\n", defIndex, (void*)weapon);
            }
        }

        // also attempt active weapon (sometimes duplicated in slots but keep safe)
        uint32_t activeHandle = Read<uint32_t>(localPlayer + m_hActiveWeapon);
        if (activeHandle)
        {
            uint32_t idx = activeHandle & 0xFFF;
            if (idx != 0 && idx <= 2048)
            {
                uintptr_t weapon = Read<uintptr_t>(clientDll + dwEntityList + (idx - 1) * 0x10);
                if (weapon)
                {
                    short defIndex = Read<short>(weapon + m_iItemDefinitionIndex);
                    if (defIndex > 0)
                    {
                        weaponsFound++;
                        auto it = skinConfig.find(defIndex);
                        if (it != skinConfig.end())
                        {
                            ApplySkinToWeapon(weapon, it->second);
                            appliedCount++;
                            printf("[SKIN] Applied to ACTIVE def %d -> paint %d weapon=0x%p\n", defIndex, it->second.paintKit, (void*)weapon);
                        }
                    }
                }
            }
        }

        printf("[SKIN] ApplySkins: weaponsFound=%d applied=%d\n", weaponsFound, appliedCount);

        // disable until next Apply click to avoid reapplying every frame
        enabled = false;
    }

    void ForceFullUpdate()
    {
        using namespace Memory;
        using namespace offsets;
        if (Memory::engineDll == 0)
        {
            printf("[SKIN] ForceFullUpdate: engineDll == 0\n");
            return;
        }
        uintptr_t clientState = Read<uintptr_t>(Memory::engineDll + dwClientState);
        if (!clientState)
        {
            printf("[SKIN] ForceFullUpdate: clientState == 0\n");
            return;
        }
        // write once to force delta
        Write<int>(clientState + dwClientState_PlayerInfo + 0x174, -1);
        printf("[SKIN] ForceFullUpdate: wrote -1 to clientState.PlayerInfo+0x174\n");
    }

    void RemoveSkin(int defIndex)
    {
        using namespace Memory;
        using namespace offsets;
        if (!Memory::clientDll) return;
        uintptr_t localPlayer = Read<uintptr_t>(Memory::clientDll + dwLocalPlayer);
        if (!localPlayer) return;
        bool any = false;
        for (int i = 0; i < 8; ++i) {
            uint32_t handle = Read<uint32_t>(localPlayer + m_hMyWeapons + i * 4);
            if (!handle) continue;
            uint32_t idx = handle & 0xFFF;
            if (idx == 0 || idx > 2048) continue;
            uintptr_t weapon = Read<uintptr_t>(Memory::clientDll + dwEntityList + (idx - 1) * 0x10);
            if (!weapon) continue;
            int curDef = Read<int>(weapon + m_iItemDefinitionIndex);
            if (curDef != defIndex) continue;
            ResetWeaponSkin(weapon);
            any = true;
        }
        if (any) {
            skinConfig.erase(defIndex);
            // do NOT force full update automatically here — leave it to the user UI
            printf("[SKIN] RemoveSkin: removed skin for defIndex %d (will require manual ForceFullUpdate to reflect)\n", defIndex);
        }
    }

    const char* GetWeaponName(int defIndex)
    {
        for (size_t i = 0; i < weaponCount; ++i)
            if (weaponIDs[i] == defIndex)
                return weaponNames[i];
        return "Unknown Weapon";
    }

    static std::unordered_map<std::string, std::string> weaponNameToToken = {
        {"AK-47", "ak47"}, {"M4A4", "m4a4"}, {"M4A1-S", "m4a1_silencer"}, {"AWP", "awp"},
        {"Glock-18", "glock"}, {"USP-S", "usp_silencer"}, {"Desert Eagle", "deagle"},
        {"P250", "p250"}, {"Five-SeveN", "fiveseven"}, {"P90", "p90"}, {"MP9", "mp9"},
        {"MP7", "mp7"}, {"MAC-10", "mac10"}, {"Nova", "nova"}, {"XM1014", "xm1014"},
        {"UMP-45", "ump45"}, {"PP-Bizon", "bizon"}, {"MAG-7", "mag7"}, {"Negev", "negev"},
        {"Sawed-Off", "sawedoff"}, {"Tec-9", "tec9"}, {"P2000", "p2000"},
        {"SCAR-20", "scar20"}, {"SG 553", "sg553"}, {"SSG 08", "ssg08"},
        {"CZ75-Auto", "cz75a"}, {"R8 Revolver", "revolver"}
    };

    void LoadSkinsDBToMenu(std::unordered_map<std::string, std::vector<std::pair<std::string, int>>>& out)
    {
        out.clear();
        for (const auto& map : weaponNameToToken)
        {
            auto it = skins_db.find(map.second);
            if (it != skins_db.end())
                out[map.first] = it->second;
        }
    }

    void SaveSkins()
    {
        // TODO: persist skinConfig to file
        printf("[SKIN] SaveSkins: not implemented\n");
    }

    void LoadSkins()
    {
        // TODO: load skinConfig from file
        printf("[SKIN] LoadSkins: not implemented\n");
    }
} // namespace SkinChanger
