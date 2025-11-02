// skinsselection.cpp
#include "SkinsSection.h"
#include "../../../Cheats/SkinChanger/SkinChanger.h"
#include "../../Fonts/IconsFontAwesome.h"
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
                vec.push_back({ sname, pid, ImVec4(0.18f, 0.18f, 0.18f, 1.0f) });
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

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 6));

        ImGui::SeparatorText(ICON_FA_PALLETE" Skin Customizer");
        ImGui::Spacing();

        // -------- STEP 0: Overview --------
        if (stage == 0)
        {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.f, 1.f), "My Equipped Skins");
            ImGui::BeginChild("##skins", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

            for (size_t i = 0; i < SkinChanger::weaponCount; ++i)
            {
                int def = SkinChanger::weaponIDs[i];
                auto it = SkinChanger::skinConfig.find(def);
                if (it == SkinChanger::skinConfig.end()) continue;

                ImGui::Separator();
                ImGui::Text("%s", SkinChanger::GetWeaponName(def));
                ImGui::SameLine();
                ImGui::TextDisabled("—");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
                    it->second.customName.empty()
                    ? ("PaintKit " + std::to_string(it->second.paintKit)).c_str()
                    : it->second.customName.c_str());
            }

            ImGui::EndChild();
            ImGui::Spacing();

            if (ImGui::Button(ICON_FA_PLUS "  Add Skin", ImVec2(ImGui::GetContentRegionAvail().x, 36)))
                stage = 1;

            ImGui::PopStyleVar(2);
            return;
        }

        // -------- STEP 1: Choose Weapon --------
        if (stage == 1)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.f, 1.f), "Step 1 / 3 — Choose Weapon");
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85); // move o botão para o lado direito
            if (ImGui::Button(ICON_FA_ARROW_LEFT "  Back", ImVec2(80, 26)))
                stage = 0;

            ImGui::Separator();
            ImGui::Spacing();

            static int combo = 0;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::Combo("##weaponCombo", &combo, SkinChanger::weaponNames, (int)SkinChanger::weaponCount);

            ImGui::Spacing();
            if (ImGui::Button("Next  " ICON_FA_ARROW_RIGHT, ImVec2(ImGui::GetContentRegionAvail().x, 34)))
            {
                selWeapon = combo;
                weaponName = SkinChanger::weaponNames[selWeapon];
                stage = 2;
            }

            ImGui::PopStyleVar(2);
            return;
        }

        // -------- STEP 2: Choose Skin --------
        if (stage == 2)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.f, 1.f), "Step 2 / 3 — Choose Skin for %s", weaponName.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85);
            if (ImGui::Button(ICON_FA_ARROW_LEFT "  Back", ImVec2(80, 26)))
                stage = 1;

            ImGui::Separator();
            ImGui::Spacing();

            auto it = skinDatabase.find(weaponName);
            ImGui::BeginChild("##skinlist", ImVec2(0, 320), true, ImGuiWindowFlags_AlwaysUseWindowPadding);
            if (it != skinDatabase.end())
            {
                int idx = 0;
                const float itemWidth = ImGui::GetContentRegionAvail().x / 3.f - 8.f;

                for (auto& s : it->second)
                {
                    ImGui::PushID(idx);
                    ImGui::PushStyleColor(ImGuiCol_Button, s.color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.f));

                    if (ImGui::Button(s.name.c_str(), ImVec2(itemWidth, 60)))
                    {
                        selSkin = idx;
                        chosen = s;

                        int def = SkinChanger::weaponIDs[selWeapon];
                        SkinChanger::skinConfig[def].paintKit = chosen.paintKit;
                        stage = 3;
                    }

                    if ((idx + 1) % 3 != 0)
                        ImGui::SameLine();

                    ImGui::PopStyleColor(3);
                    ImGui::PopID();
                    idx++;
                }
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(2);
            return;
        }

        // -------- STEP 3: Customize --------
        if (stage == 3)
        {
            int def = SkinChanger::weaponIDs[selWeapon];
            auto& cfg = SkinChanger::skinConfig[def];

            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.f, 1.f), "Step 3 / 3 — Customize %s", weaponName.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 85);
            if (ImGui::Button(ICON_FA_ARROW_LEFT "  Back", ImVec2(80, 26)))
                stage = 2;

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::SliderFloat("Wear", &cfg.wear, 0.f, 1.f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::Checkbox("StatTrak Enabled", &cfg.statTrak);
            if (cfg.statTrak)
                ImGui::InputInt("StatTrak Count", &cfg.statTrakCount);

            static char buf[64] = {};
            if (ImGui::InputText("Custom Name", buf, 64))
                cfg.customName = buf;

            ImGui::Spacing();
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

            ImGui::PopStyleVar(2);
        }
    }


}