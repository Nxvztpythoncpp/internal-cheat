#include "SkinChanger.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <Windows.h>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include "../../Gui/Menu/Skins/skins_db.h"

namespace SkinChanger
{
    std::unordered_map<int, WeaponSkin> skinConfig;

    struct ActiveWeaponState {
        uintptr_t weapon = 0;
        float firstSeenTime = 0.f;
        bool applied = false;
    };

    static std::unordered_map<uintptr_t, ActiveWeaponState> activeWeapons;
    static std::atomic<bool> skinThreadRunning = false;
    static std::thread skinThread;
    constexpr size_t MAX_CUSTOM_NAME = 32;

    static float GetTimeSeconds()
    {
        using namespace std::chrono;
        return duration_cast<duration<float>>(steady_clock::now().time_since_epoch()).count();
    }

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

        Memory::Write<int>(weapon + offsets::m_nFallbackPaintKit, std::clamp(skin.paintKit, 0, 200000));
        Memory::Write<float>(weapon + offsets::m_flFallbackWear, std::clamp(skin.wear, 0.0f, 1.0f));
        Memory::Write<int>(weapon + offsets::m_nFallbackSeed, skin.seed);
        Memory::Write<int>(weapon + offsets::m_nFallbackStatTrak, skin.statTrak ? skin.statTrakCount : -1);
        Memory::Write<int>(weapon + offsets::m_iItemIDHigh, -1);
        Memory::Write<int>(weapon + offsets::m_iEntityQuality, 3);
        WriteCustomName(weapon, skin.customName);

        uintptr_t clientState = Memory::Read<uintptr_t>(Memory::engineDll + offsets::dwClientState);
        if (clientState)
            Memory::Write<int>(clientState + offsets::dwClientState_PlayerInfo + 0x174, -1);

        DebugPrint("[SkinChanger] Applied skin %d to weapon at 0x%p", skin.paintKit, (void*)weapon);
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

        for (size_t i = 0; i < 32; ++i)
            Write<char>(weapon + m_szCustomName + i, '\0');

        DebugPrint("[SkinChanger] Reset weapon skin at 0x%p", (void*)weapon);
    }

    void ForceFullUpdate()
    {
        using namespace Memory;
        using namespace offsets;
        if (!Memory::engineDll) return;
        uintptr_t clientState = Read<uintptr_t>(Memory::engineDll + dwClientState);
        if (!clientState) return;
        Write<int>(clientState + dwClientState_PlayerInfo + 0x174, -1);
        DebugPrint("[SkinChanger] ForceFullUpdate sent");
    }

    void RemoveSkin(int defIndex)
    {
        using namespace Memory;
        using namespace offsets;

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
            DebugPrint("[SkinChanger] Removed skin for weapon defIndex %d", defIndex);
            ForceFullUpdate();
        }
    }

    const char* GetWeaponName(int defIndex)
    {
        for (size_t i = 0; i < weaponCount; ++i)
            if (weaponIDs[i] == defIndex)
                return weaponNames[i];
        return "Unknown Weapon";
    }

    static void SkinApplyThread()
    {
        while (skinThreadRunning)
        {
            if (!enabled) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            uintptr_t localPlayer = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            bool anyChanged = false;

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

                auto& wstate = activeWeapons[weapon];

                // always apply if weapon is new or config changed
                bool shouldApply = wstate.weapon != weapon || !wstate.applied;
                if (shouldApply)
                {
                    ApplySkinToWeapon(weapon, it->second);
                    wstate.weapon = weapon;
                    wstate.applied = true;
                    anyChanged = true;
                }
            }

            if (anyChanged)
                ForceFullUpdate();

            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // tiny sleep to prevent 100% CPU
        }
    }


    void Run()
    {
        if (enabled && !skinThreadRunning)
        {
            skinThreadRunning = true;
            skinThread = std::thread(SkinApplyThread);
            DebugPrint("[SkinChanger] Skin thread started");
        }
        else if (!enabled && skinThreadRunning)
        {
            skinThreadRunning = false;
            if (skinThread.joinable())
                skinThread.join();
            DebugPrint("[SkinChanger] Skin thread stopped");
        }
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

    void SaveSkins() {}
    void LoadSkins() {}
}
