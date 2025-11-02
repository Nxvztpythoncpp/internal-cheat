#include "RagebotSection.h"
#include "../../../Cheats/Rage/Aimbot.h"

void Menu::DrawRagebotSection(){

    ImGui::Text("Ragebot Settings");

    static bool aimbot = Aimbot::enabled;
    static float fov = Aimbot::fov;
    static bool draw_fov_circle = Aimbot::draw_fov_circle;
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

    ImGui::SliderFloat("FOV", &fov, 1.0f, 180.0f, "%.1f");
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

    const char* aimKeys[] = { "Always", "Left Mouse (LMB)", "Right Mouse (RMB)", "Middle Mouse (MMB)", "Mouse X1", "Mouse X2" };

    int aimKeyIndex = 0;
    switch (Aimbot::aim_key) {
    case 1: aimKeyIndex = 1; break; // LMB
    case 2: aimKeyIndex = 2; break; // RMB
    case 4: aimKeyIndex = 3; break; // MMB
    case 5: aimKeyIndex = 4; break; // X1
    case 6: aimKeyIndex = 5; break; // X2
    default: aimKeyIndex = 0; break; // Always
    }

    ImGui::Combo("Aim Key", &aimKeyIndex, aimKeys, IM_ARRAYSIZE(aimKeys));

    int aimKeyValues[] = { 0, 1, 2, 4, 5, 6 };
    Aimbot::aim_key = aimKeyValues[aimKeyIndex];

    ImGui::SliderInt("Aim Bone", &aim_bone, 0, 128);
    Aimbot::aim_bone = aim_bone;

    ImGui::SliderFloat("Smooth", &smooth_factor, 1.0f, 20.0f, "%.1f");
    Aimbot::smooth_factor = smooth_factor;

    if (ImGui::Checkbox("Draw FOV Circle", &draw_fov_circle)) {
        Aimbot::draw_fov_circle = draw_fov_circle;
    }
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("lmao test");

    ImGui::Text("Aim Key: %d (0=Always, 1=LMB, 2=RMB, 4=MMB, 5=X1, 6=X2)", aim_key);
    ImGui::Text("Bones: 8=Head, 6=Neck, 5=Chest");
}
