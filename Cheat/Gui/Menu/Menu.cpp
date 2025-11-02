#define _CRT_SECURE_NO_WARNINGS

#include "Menu.h"
#include "StyleColors.h"

#include "../Imgui/imgui.h"
#include "../Imgui/ImGuiHelpers.h"
#include "../Fonts/IconsFontAwesome.h"
#include "../Fonts/Fonts.h"

#include "../../Dependencies/Hooks/Hooks.h"

#include "Ragebot/RagebotSection.h"
#include "AntiAim/AntiAim.h"
#include "Visuals/VisualsSection.h"
#include "Misc/MiscSection.h"
#include "Skins/SkinsSection.h"
#include "Configs/ConfigsSection.h"
#include "Lua/LuasSection.h"
#include "RenderUtils.h"

namespace Menu {

    struct Stats {
        int totalKills = 67;
        int totalDeaths = 67;
        int totalAssists = 67;
        int currentKills = 67;
        int currentDeaths = 67;
        int currentAssists = 67;

        float GetTotalKD() const {
            return totalDeaths > 0 ? (float)totalKills / totalDeaths : (float)totalKills;
        }

        float GetCurrentKD() const {
            return currentDeaths > 0 ? (float)currentKills / currentDeaths : (float)currentKills;
        }
    };

    static Stats playerStats;

    enum class SidebarSection {
        Stats,
        Ragebot,
        AntiAim,
        Skins,
        Visuals,
        Misc,
        Configs,
        Luas
    };

    static SidebarSection currentSection = SidebarSection::Stats;
    static int currentTab = 0; // 0 = Local, 1 = Players

    void DrawSidebarButton(const char* label, SidebarSection section) {
        ImVec2 size = ImVec2(120, 35);
        bool active = (currentSection == section);
        ImVec4 activeColor = ImVec4(0.3f, 0.1f, 0.6f, 1.0f);

        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.2f, 0.7f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
        }

        if (ImGui::Button(label, size)) {
            currentSection = section;
        }

        if (active) {
          //  ImGui::PopStyleColor(3);
        }
    }

    void DrawStatsBox(const char* title, float kd, int kills, int deaths, int assists) {
        ImGui::BeginChild(title, ImVec2(200, 140), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Text("%s", title);
        ImGui::Separator();
        ImGui::Text("KD total: %.2f", kd);
        ImGui::Text("Kills: %d", kills);
        ImGui::Text("Deaths: %d", deaths);
        ImGui::Text("Assists: %d", assists);
        ImGui::EndChild();
    }

    void DrawKDGraph() {
        ImGui::BeginChild("KD Graph", ImVec2(0, 140), true);
        ImGui::Text("Graph KD each day for 1 week");
        ImGui::Separator();

        static float kd_values[7] = { 1.2f, 1.5f, 1.3f, 1.8f, 1.6f, 1.4f, 1.7f };
        static const char* days[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

        ImGui::PlotLines("##kd_graph", kd_values, IM_ARRAYSIZE(kd_values), 0, nullptr, 0.5f, 2.5f, ImVec2(-1, 80));

        ImGui::Columns(7, "##days", false);
        for (int i = 0; i < 7; i++) {
            ImGui::Text("%s", days[i]);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild();
    }

    void DrawStatsSection()
    {
        if (BeginShadowedChild("TopTabsCard", ImVec2(0, 48)))
        {
            if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyResizeDown))
            {
                if (ImGui::BeginTabItem("Local")) { currentTab = 0; ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Players")) { currentTab = 1; ImGui::EndTabItem(); }
                ImGui::EndTabBar();
            }
            EndShadowedChild();
        }

        ImGui::Spacing();

        if (BeginShadowedChild("StatsBoxesCard", ImVec2(0, 170)))
        {
            ImGui::Columns(2, "##stats_columns", false);
            DrawStatsBox("Total stats", playerStats.GetTotalKD(), playerStats.totalKills, playerStats.totalDeaths, playerStats.totalAssists);
            ImGui::NextColumn();
            DrawStatsBox("Current stats", playerStats.GetCurrentKD(), playerStats.currentKills, playerStats.currentDeaths, playerStats.currentAssists);
            ImGui::Columns(1);
            EndShadowedChild();
        }

        ImGui::Spacing();

        if (BeginShadowedChild("KDGraphCard", ImVec2(0, 160)))
        {
            DrawKDGraph();
            EndShadowedChild();
        }
    }

    void Draw() {

        static bool fonts_initialized = false;

        if (!Hooks::menu_open) return;

        InitFonts();

        ImGui::PushFont(GetIconFont());


        StyleColors::Apply();
        ImGui::SetNextWindowSize(ImVec2(850, 500), ImGuiCond_FirstUseEver);

        ImGui::Begin("##MainWindow", nullptr,
            ImGuiWindowFlags_NoTitleBar |          
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImRect wrect = ImGui::GetCurrentWindow()->Rect();
        AddShadowRect(dl, wrect, ImGui::GetStyle().WindowRounding, IM_COL32(0, 0, 0, 180), 7, 28.0f);

        {
            ImVec2 headerMin(wrect.Min.x, wrect.Min.y);
            ImVec2 headerMax(wrect.Max.x, wrect.Min.y + 56.0f);
            ImU32 colA = IM_COL32(120, 45, 215, 220); 
            ImU32 colB = IM_COL32(220, 90, 200, 220); 

            float rounding = ImGui::GetStyle().WindowRounding; 

            //gradient
            //dl->AddRectFilled(headerMin, headerMax, colA, rounding, ImDrawFlags_RoundCornersTop);
            //dl->AddRectFilledMultiColor(headerMin, headerMax, colA, colB, colB, colA); 

            dl->AddLine(ImVec2(headerMin.x + 8, headerMax.y - 1), ImVec2(headerMax.x - 8, headerMax.y - 1), IM_COL32(255, 255, 255, 18), 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(headerMin.x + 20, headerMin.y + 16));
            ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.98f, 1.0f), "Cyclone.cc");

            ImVec2 avatarPos(headerMax.x - 64, headerMin.y + 8);
            float avatarSize = 40.0f;
            dl->AddRectFilled(ImVec2(avatarPos.x, avatarPos.y), ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), IM_COL32(22, 22, 30, 255), 8.0f);

            ImVec2 buttonPos = ImVec2(avatarPos.x + 12, avatarPos.y + 10);
            ImGui::SetCursorScreenPos(buttonPos);

            ImVec2 buttonSize = ImVec2(20, 20);

            if (ImGui::InvisibleButton("settingsButton", buttonSize)) {
                ImGui::OpenPopup("SettingsWindow");
            }

            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddText(buttonPos, IM_COL32(240, 240, 255, 255), ICON_FA_GEAR);

            if (ImGui::BeginPopup("SettingsWindow")) {
                ImGui::Text("Settings here");
                ImGui::Text("Injection timer");
                ImGui::Text("Menu personalization");
                ImGui::EndPopup();
            }

        }

        ImGui::Spacing();
        ImGui::Dummy(ImVec2(0, 2));

        // === Sidebar ===
        ImGui::BeginChild("Sidebar", ImVec2(150, 0), true);
        ImGui::Dummy(ImVec2(0, 12));

        if (SidebarIconButton(ICON_FA_CHART_SIMPLE, "Stats", currentSection == SidebarSection::Stats, ImVec2(130, 36))) currentSection = SidebarSection::Stats;

        ImGui::Separator();

        if (SidebarIconButton(ICON_FA_CROSSHAIRS, "Ragebot", currentSection == SidebarSection::Ragebot, ImVec2(130, 36))) currentSection = SidebarSection::Ragebot;
        if (SidebarIconButton(ICON_FA_SHIELD_HALVED, "Anti Aim", currentSection == SidebarSection::AntiAim, ImVec2(130, 36))) currentSection = SidebarSection::AntiAim;
        if (SidebarIconButton(ICON_FA_PEN_FANCY, "Skins", currentSection == SidebarSection::Skins, ImVec2(130, 36))) currentSection = SidebarSection::Skins;
        if (SidebarIconButton(ICON_FA_EYE, "Visuals", currentSection == SidebarSection::Visuals, ImVec2(130, 36))) currentSection = SidebarSection::Visuals;
        if (SidebarIconButton(ICON_FA_WAND_MAGIC, "Misc", currentSection == SidebarSection::Misc, ImVec2(130, 36))) currentSection = SidebarSection::Misc;

        ImGui::Separator();

        if (SidebarIconButton(ICON_FA_CLOUD, "Configs", currentSection == SidebarSection::Configs, ImVec2(130, 36))) currentSection = SidebarSection::Configs;
        if (SidebarIconButton(ICON_FA_MOON, "Luas", currentSection == SidebarSection::Luas, ImVec2(130, 36))) currentSection = SidebarSection::Luas;

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("MainContent", ImVec2(0, 0), false);

        switch (currentSection) {
        case SidebarSection::Stats: DrawStatsSection(); break;
        case SidebarSection::Ragebot: DrawRagebotSection(); break;
        case SidebarSection::AntiAim: DrawAntiAimSection(); break;
        case SidebarSection::Skins: DrawSkinChangerTab(); break;
        case SidebarSection::Visuals: DrawVisualsSection(); break;
        case SidebarSection::Misc: DrawMiscSection(); break;
        case SidebarSection::Configs: DrawConfigsSection(); break;
        case SidebarSection::Luas: DrawLuasSection(); break;
        }

        ImGui::EndChild();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            Hooks::menu_open = false;
        }
        DrawFovCircle();
        ImGui::End();
        StyleColors::Restore();

        ImGui::PopFont();
    }
}
