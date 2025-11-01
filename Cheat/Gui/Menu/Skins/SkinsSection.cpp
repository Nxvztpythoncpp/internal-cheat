// skinsselection.cpp
#include "SkinsSection.h"
#include "../../../Cheats/SkinChanger/SkinChanger.h"
#include "../../../Memory/Memory.h"
#include "../../../Memory/Offsets.h"
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>

namespace Menu
{
    struct SkinOption {
        std::string name;
        int paintKit;
        ImVec4 color;
    };

    static std::unordered_map<std::string, std::vector<SkinOption>> skinDatabase;

    static void PopulateSkins()
    {
        std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> temp;
        SkinChanger::LoadSkinsDBToMenu(temp);
        skinDatabase.clear();

        for (auto& [wname, list] : temp)
        {
            std::vector<SkinOption> vec;
            for (auto& [sname, pid] : list)
                vec.push_back({ sname, pid, ImVec4(0.8f, 0.8f, 0.8f, 1.0f) });
            skinDatabase[wname] = std::move(vec);
        }
    }

    static void ForceUpdate()
    {
        uintptr_t cs = Memory::Read<uintptr_t>(Memory::engineDll + offsets::dwClientState);
        if (cs)
            Memory::Write<int>(cs + offsets::dwClientState_PlayerInfo + 0x174, -1);
    }

    void DrawSkinChangerTab()
    {
        static bool initialized = false;
        if (!initialized) { PopulateSkins(); initialized = true; }

        static int stage = 0;
        static int selWeapon = -1;
        static std::string weaponName;
        static SkinOption chosen;
        static int selSkin = -1;

        ImGui::SeparatorText("My Skins");

        if (stage == 0)
        {
            ImGui::BeginChild("##skins", ImVec2(0, 300), true);
            for (size_t i = 0; i < SkinChanger::weaponCount; ++i)
            {
                int def = SkinChanger::weaponIDs[i];
                auto it = SkinChanger::skinConfig.find(def);
                if (it == SkinChanger::skinConfig.end()) continue;
                ImGui::Text("%s — %s", SkinChanger::GetWeaponName(def),
                    it->second.customName.empty() ?
                    ("PaintKit " + std::to_string(it->second.paintKit)).c_str() :
                    it->second.customName.c_str());
            }
            ImGui::EndChild();
            if (ImGui::Button("➕ Add Skin", ImVec2(ImGui::GetContentRegionAvail().x, 32)))
                stage = 1;
            return;
        }

        if (stage == 1)
        {
            static int combo = 0;
            ImGui::Text("Step 1/3 — Choose Weapon");
            ImGui::Combo("Weapon", &combo, SkinChanger::weaponNames, (int)SkinChanger::weaponCount);
            if (ImGui::Button("Next →", ImVec2(ImGui::GetContentRegionAvail().x, 30)))
            {
                selWeapon = combo;
                weaponName = SkinChanger::weaponNames[selWeapon];
                stage = 2;
            }
            if (ImGui::Button("← Back")) stage = 0;
            return;
        }

        if (stage == 2)
        {
            ImGui::Text("Step 2/3 — Choose Skin for %s", weaponName.c_str());
            auto it = skinDatabase.find(weaponName);
            ImGui::BeginChild("##skins", ImVec2(0, 300), true);
            if (it != skinDatabase.end())
            {
                int idx = 0;
                ImGui::Columns(3, nullptr, false);
                for (auto& s : it->second)
                {
                    ImGui::PushID(idx);
                    ImGui::PushStyleColor(ImGuiCol_Button, s.color);
                    if (ImGui::Button(s.name.c_str(), ImVec2(-FLT_MIN, 60)))
                    {
                        selSkin = idx;
                        chosen = s;
                    }
                    ImGui::PopStyleColor();
                    if (++idx % 3 != 0) ImGui::NextColumn();
                    ImGui::PopID();
                }
                ImGui::Columns(1);
            }
            ImGui::EndChild();

            if (ImGui::Button("Next →", ImVec2(ImGui::GetContentRegionAvail().x, 30)))
            {
                int def = SkinChanger::weaponIDs[selWeapon];
                SkinChanger::skinConfig[def].paintKit = chosen.paintKit;
                stage = 3;
            }
            if (ImGui::Button("← Back")) stage = 1;
            return;
        }

        if (stage == 3)
        {
            int def = SkinChanger::weaponIDs[selWeapon];
            auto& cfg = SkinChanger::skinConfig[def];

            ImGui::Text("Step 3/3 — Customize");
            ImGui::InputFloat("Wear", &cfg.wear, 0.01f, 0.1f, "%.4f");
            cfg.wear = std::clamp(cfg.wear, 0.f, 1.f);
            ImGui::Checkbox("StatTrak", &cfg.statTrak);
            if (cfg.statTrak)
                ImGui::InputInt("Count", &cfg.statTrakCount);
            static char buf[64] = {};
            if (ImGui::InputText("Custom Name", buf, 64))
                cfg.customName = buf;

            if (ImGui::Button("Apply & Return", ImVec2(ImGui::GetContentRegionAvail().x, 36)))
            {
                if (!SkinChanger::enabled) SkinChanger::enabled = true;
                uintptr_t local = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
                if (local)
                {
                    for (int i = 0; i < 8; ++i)
                    {
                        uint32_t h = Memory::Read<uint32_t>(local + offsets::m_hMyWeapons + i * 4);
                        if (!h) continue;
                        uint32_t idx = h & 0xFFF;
                        uintptr_t w = Memory::Read<uintptr_t>(Memory::clientDll + offsets::dwEntityList + (idx - 1) * 0x10);
                        if (!w) continue;
                        int curDef = Memory::Read<int>(w + offsets::m_iItemDefinitionIndex);
                        if (curDef != def) continue;
                        SkinChanger::ApplySkinToWeapon(w, cfg);
                    }
                }
                ForceUpdate();
                stage = 0;
            }
            if (ImGui::Button("← Back")) stage = 2;
        }
    }
}
