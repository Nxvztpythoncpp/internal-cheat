#include "VisualsSection.h"
#include "../../../Cheats/Visuals/Visuals.h"


void Menu::DrawVisualsSection() {
    ImGui::Text("Visuals Settings");

    static bool esp = Visuals::espEnabled;
    static bool glow = Visuals::glowEnabled;
    static bool glowWalls = Visuals::glowThroughWalls;
    static bool healthBar = Visuals::drawHealthBar;
    static bool boxes = Visuals::drawBoxes;
    static bool skeleton = Visuals::drawSkeleton;
    static bool teamChk = Visuals::teamCheck;
    static bool& boxOut = Visuals::boxOutline;

    ImGui::Checkbox("ESP", &esp);

    if (ImGui::Checkbox("Glow", &glow)) {
        Visuals::glowEnabled = glow;
    }

    if (ImGui::Checkbox("Glow Through Walls", &glowWalls)) {
        Visuals::glowThroughWalls = glowWalls;
    }

    ImGui::ColorEdit4("Glow Enemy Color", Visuals::glowColorEnemy);
    ImGui::ColorEdit4("Glow Team Color", Visuals::glowColorTeam);

    if (ImGui::Checkbox("Health Bar", &healthBar)) {
        Visuals::drawHealthBar = healthBar;
    }

    if (ImGui::Checkbox("Box ESP", &boxes)) {
        Visuals::drawBoxes = boxes;
    }

    ImGui::Checkbox("Box Outline", &boxOut);

    ImGui::ColorEdit4("Box Color", Visuals::boxColor);
    if (ImGui::Checkbox("Team Check", &teamChk)) {
        Visuals::teamCheck = teamChk;
    }

    ImGui::ColorEdit4("Skeleton Color", Visuals::skeletonColor);
    if (ImGui::Checkbox("Skeleton ESP", &skeleton)) {
        Visuals::drawSkeleton = skeleton;
    }

}