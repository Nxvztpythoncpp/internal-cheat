#pragma once
#include <string>
#include <unordered_map>
#include <Windows.h>

namespace SkinChanger
{
    struct WeaponSkin
    {
        int paintKit = 0;
        int seed = 0;
        float wear = 0.0001f;
        bool statTrak = false;
        int statTrakCount = 0;
        std::string customName = "";
    };

    inline bool enabled = false;

    // declaração externa — a definição fica em SkinChanger.cpp
    extern std::unordered_map<int, WeaponSkin> skinConfig;

    inline const int weaponIDs[] = {
        // Pistols & heavy
        1, 2, 3, 4, 7, 8, 9, 10, 11, 13, 14, 16, 17, 19, 24,
        25, 26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 38, 39, 40,
        60, 61, 63, 64,
        // Default knives
        42, 59,
        // Premium knives
        500, 505, 506, 507, 508, 509, 512, 514, 515, 516,
        519, 520, 522, 523
    };

    inline const char* weaponNames[] = {
        "Desert Eagle","Dual Berettas","Five-SeveN","Glock-18",
        "AK-47","AUG","AWP","FAMAS","G3SG1","Galil AR","M249",
        "M4A4","MAC-10","P90","UMP-45","XM1014",
        "PP-Bizon","MAG-7","Negev","Sawed-Off","Tec-9","P2000",
        "MP7","MP9","Nova","P250","SCAR-20","SG 553","SSG 08",
        "M4A1-S","USP-S","CZ75-Auto","R8 Revolver",
        "Knife (CT)","Knife (T)",
        "Bayonet","Flip Knife","Gut Knife","Karambit","M9 Bayonet",
        "Huntsman Knife","Falchion Knife","Bowie Knife","Butterfly Knife",
        "Shadow Daggers","Ursus Knife","Navaja Knife","Stiletto Knife","Talon Knife"
    };

    inline constexpr size_t weaponCount = sizeof(weaponIDs) / sizeof(weaponIDs[0]);

    void ApplySkinToWeapon(uintptr_t weapon, const WeaponSkin& skin);
    void Run();
    void SaveSkins();
    void LoadSkins();
    const char* GetWeaponName(int defIndex);

    // nova função pública para popular o menu com o skins_db.h gerado
    void LoadSkinsDBToMenu(std::unordered_map<std::string, std::vector<std::pair<std::string, int>>>& out);

    inline void DebugPrint(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        printf("[SkinChanger] ");
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }

    void WriteCustomName(uintptr_t weapon, const std::string& name);
}
