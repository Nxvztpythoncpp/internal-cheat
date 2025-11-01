// SkinChanger.cpp
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
#include <mutex>
#include <cstdio>
#include "../../Gui/Menu/Skins/skins_db.h" // assume este header fornece skins_db

namespace SkinChanger
{
    // NOTE:
    // - Não redeclara WeaponSkin (está em SkinChanger.h).
    // - Não declara extern bool enabled / weaponIDs / weaponNames / weaponCount
    //   porque eles já são definidos como `inline` no header.
    // - Aqui definimos a única definição do skinConfig (declarada extern no .h).

    std::unordered_map<int, WeaponSkin> skinConfig;

    struct ActiveWeaponState {
        uintptr_t weapon = 0;
        float firstSeenTime = 0.f;
        bool applied = false;
    };

    static std::unordered_map<uintptr_t, ActiveWeaponState> activeWeapons;
    static std::mutex activeWeaponsMutex;

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

        // Escrever campos principais (offsets do teu Offsets.h)
        Memory::Write<int>(weapon + offsets::m_nFallbackPaintKit, std::clamp(skin.paintKit, 0, 200000));
        Memory::Write<float>(weapon + offsets::m_flFallbackWear, std::clamp(skin.wear, 0.0f, 1.0f));
        Memory::Write<int>(weapon + offsets::m_nFallbackSeed, skin.seed);
        Memory::Write<int>(weapon + offsets::m_nFallbackStatTrak, skin.statTrak ? skin.statTrakCount : -1);
        Memory::Write<int>(weapon + offsets::m_iItemIDHigh, -1);
        Memory::Write<int>(weapon + offsets::m_iEntityQuality, 3);
        WriteCustomName(weapon, skin.customName);

        // Forçar atualização do ClientState para aplicar mudanças visualmente
        if (Memory::engineDll) {
            uintptr_t clientState = Memory::Read<uintptr_t>(Memory::engineDll + offsets::dwClientState);
            if (clientState)
                Memory::Write<int>(clientState + offsets::dwClientState_PlayerInfo + 0x174, -1);
        }

        //DebugPrint("Applied skin paintkit=%d to weapon 0x%p", skin.paintKit, (void*)weapon);
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

        // Limpar nome personalizado
        for (size_t i = 0; i < MAX_CUSTOM_NAME; ++i)
            Write<char>(weapon + m_szCustomName + i, '\0');

        //DebugPrint("Reset weapon skin at 0x%p", (void*)weapon);
    }

    void ForceFullUpdate()
    {
        using namespace Memory;
        using namespace offsets;
        if (!Memory::engineDll) return;
        uintptr_t clientState = Read<uintptr_t>(Memory::engineDll + dwClientState);
        if (!clientState) return;
        Write<int>(clientState + dwClientState_PlayerInfo + 0x174, -1);
        //DebugPrint("ForceFullUpdate sent");
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
            //DebugPrint("Removed skin for weapon defIndex %d", defIndex);
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

    // Thread que aplica skins
    static void SkinApplyThread()
    {
        using namespace std::chrono_literals;

        while (skinThreadRunning)
        {
            if (!enabled) {
                std::this_thread::sleep_for(500ms);
                continue;
            }

            uintptr_t localPlayer = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) {
                std::this_thread::sleep_for(100ms);
                continue;
            }

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
                float timeNow = GetTimeSeconds();

                if (wstate.weapon != weapon)
                {
                    wstate.weapon = weapon;
                    wstate.firstSeenTime = timeNow;
                    wstate.applied = false;
                }

                if (!wstate.applied || (timeNow - wstate.firstSeenTime) < 5.f)
                {
                    ApplySkinToWeapon(weapon, it->second);
                    wstate.applied = true;
                }
            }
        }
    }

    void Run()
    {
        if (enabled && !skinThreadRunning)
        {
            skinThreadRunning = true;
            skinThread = std::thread(SkinApplyThread);
            //DebugPrint("Skin thread started");
        }
        else if (!enabled && skinThreadRunning)
        {
            skinThreadRunning = false;
            if (skinThread.joinable())
                skinThread.join();
            //DebugPrint("Skin thread stopped");
        }
    }

    // Mapeamento token->nome (para UI)
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

    // Simplified save/load (stub) - integrate com a tua persistência se necessário
    void SaveSkins()
    {
        // TODO: serializar skinConfig para ficheiro se necessário
        //DebugPrint("SaveSkins called (not implemented)");
    }

    void LoadSkins()
    {
        // TODO: desserializar skinConfig de ficheiro se necessário
        //DebugPrint("LoadSkins called (not implemented)");
    }
} // namespace SkinChanger
