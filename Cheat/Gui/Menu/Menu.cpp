#include "Menu.h"
#include "StyleColors.h"
#include <Windows.h>
#include "../Imgui/imgui.h"
#include "../../Dependencies/Hooks/Hooks.h"
#include <string>
#include "../../Cheats/Rage/Aimbot.h"
#include "../Imgui/ImGuiHelpers.h"
#include "../../Cheats/Visuals/Visuals.h"

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
            ImGui::PopStyleColor(3);
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


    void DrawRagebotSection() {
        ImGui::Text("Ragebot Settings");

        static bool aimbot = Aimbot::enabled;
        static float fov = Aimbot::fov;
        static bool silent = Aimbot::silent;
        static bool rcs = Aimbot::rcs;
        static bool visibility_check = Aimbot::visibility_check;
        static bool team_check = Aimbot::team_check;
        static int aim_key = Aimbot::aim_key;
        static int aim_bone = Aimbot::aim_bone;
        static float smooth_factor = Aimbot::smooth_factor;

        if (ImGui::Checkbox("Aimbot", &aimbot)) {
            Aimbot::enabled = aimbot;
        }

        ImGui::SliderFloat("FOV", &fov, 1.0f, 10.0f, "%.1f");
        Aimbot::fov = fov;

        if (ImGui::Checkbox("Silent Aim", &silent)) {
            Aimbot::silent = silent;
        }

        if (ImGui::Checkbox("RCS", &rcs)) {
            Aimbot::rcs = rcs;
        }

        if (ImGui::Checkbox("Visibility Check", &visibility_check)) {
            Aimbot::visibility_check = visibility_check;
        }

        if (ImGui::Checkbox("Team Check", &team_check)) {
            Aimbot::team_check = team_check;
        }

        ImGui::SliderInt("Aim Key", &aim_key, 0, 255);
        Aimbot::aim_key = aim_key;

        ImGui::SliderInt("Aim Bone", &aim_bone, 0, 128);
        Aimbot::aim_bone = aim_bone;

        ImGui::SliderFloat("Smooth", &smooth_factor, 1.0f, 20.0f, "%.1f");
        Aimbot::smooth_factor = smooth_factor;

        ImGui::Text("Aim Key: %d (0=Always, 1=LMB, 2=RMB, 4=MMB, 5=X1, 6=X2)", aim_key);
        ImGui::Text("Bones: 8=Head, 6=Neck, 5=Chest");
    }

    void DrawAntiAimSection() {
        ImGui::Text("Anti Aim Settings");
        static bool enabled = false;
        static float pitch = 0.0f;
        ImGui::Checkbox("Enabled", &enabled);
        ImGui::SliderFloat("Pitch", &pitch, -90.0f, 90.0f, "%.1f");
    }

    void DrawVisualsSection() {
        ImGui::Text("Visuals Settings");
        static bool esp = true;
        static bool glow = Visuals::glowEnabled;
        static bool glowWalls = Visuals::glowThroughWalls;

        // ESP checkbox (placeholder for future ESP feature)
        ImGui::Checkbox("ESP", &esp);

        // Glow checkbox
        if (ImGui::Checkbox("Glow", &glow)) {
            Visuals::glowEnabled = glow;
        }
        // Glow through walls checkbox
        if (ImGui::Checkbox("Glow Through Walls", &glowWalls)) {
            Visuals::glowThroughWalls = glowWalls;
        }
        // Color picker for glow (RGBA)
        ImGui::ColorEdit4("Glow Color", Visuals::glowColor);
    }

    void DrawMiscSection() {
        ImGui::Text("Misc Settings");
        static bool bhop = true;
        static bool bunnyhop = true;
        static bool radar = false;
        ImGui::Checkbox("Bunny Hop", &bhop);
        ImGui::Checkbox("Perfect Bunnyhop", &bunnyhop);
        ImGui::Checkbox("Radar Hack", &radar);
    }

    void DrawConfigsSection() {
        ImGui::Text("Configs");
        if (ImGui::Button("Legit", ImVec2(80, 30))) {}
        ImGui::SameLine();
        if (ImGui::Button("Rage", ImVec2(80, 30))) {}
        ImGui::SameLine();
        if (ImGui::Button("HvH", ImVec2(80, 30))) {}
    }

    void DrawLuasSection() {
        ImGui::Text("Luas");
        ImGui::SameLine();
        if (ImGui::SmallButton("Refresh")) {}

        static const char* luas[] = { "Aimbot.lua", "ESP.lua", "Misc.lua" };
        static int selected_lua = -1;
        for (int i = 0; i < IM_ARRAYSIZE(luas); i++) {
            if (ImGui::Selectable(luas[i], selected_lua == i)) {
                selected_lua = i;
            }
        }
    }

    void Draw() {
        if (!Hooks::menu_open) return;

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
            dl->AddRectFilled(headerMin, headerMax, colA, rounding, ImDrawFlags_RoundCornersTop);
            dl->AddRectFilledMultiColor(headerMin, headerMax, colA, colB, colB, colA); 

            dl->AddLine(ImVec2(headerMin.x + 8, headerMax.y - 1), ImVec2(headerMax.x - 8, headerMax.y - 1), IM_COL32(255, 255, 255, 18), 1.0f);

            ImGui::SetCursorScreenPos(ImVec2(headerMin.x + 20, headerMin.y + 16));
            ImGui::TextColored(ImVec4(0.96f, 0.96f, 0.98f, 1.0f), "CS:GO Menu");

            ImVec2 avatarPos(headerMax.x - 64, headerMin.y + 8);
            float avatarSize = 40.0f;
            dl->AddRectFilled(ImVec2(avatarPos.x, avatarPos.y), ImVec2(avatarPos.x + avatarSize, avatarPos.y + avatarSize), IM_COL32(22, 22, 30, 255), 8.0f);
            dl->AddText(ImVec2(avatarPos.x + 12, avatarPos.y + 10), IM_COL32(240, 240, 255, 255), "A");
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
        if (SidebarIconButton(ICON_FA_EYE, "Visuals", currentSection == SidebarSection::Visuals, ImVec2(130, 36))) currentSection = SidebarSection::Visuals;
        if (SidebarIconButton(ICON_FA_WAND_MAGIC, "Misc", currentSection == SidebarSection::Misc, ImVec2(130, 36))) currentSection = SidebarSection::Misc;

        ImGui::Separator();

        if (SidebarIconButton(ICON_FA_GEAR, "Configs", currentSection == SidebarSection::Configs, ImVec2(130, 36))) currentSection = SidebarSection::Configs;
        if (SidebarIconButton(ICON_FA_CODE, "Luas", currentSection == SidebarSection::Luas, ImVec2(130, 36))) currentSection = SidebarSection::Luas;

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("MainContent", ImVec2(0, 0), false);

        switch (currentSection) {
        case SidebarSection::Stats: DrawStatsSection(); break;
        case SidebarSection::Ragebot: DrawRagebotSection(); break;
        case SidebarSection::AntiAim: DrawAntiAimSection(); break;
        case SidebarSection::Visuals: DrawVisualsSection(); break;
        case SidebarSection::Misc: DrawMiscSection(); break;
        case SidebarSection::Configs: DrawConfigsSection(); break;
        case SidebarSection::Luas: DrawLuasSection(); break;
        }

        ImGui::EndChild();

        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            Hooks::menu_open = false;
        }

        ImGui::End();
        StyleColors::Restore();
    }
}
