#include "AntiAim.h"

void Menu::DrawAntiAimSection() {
    ImGui::Text("Anti Aim Settings");
    static bool enabled = false;
    static float pitch = 0.0f;
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::SliderFloat("Pitch", &pitch, -90.0f, 90.0f, "%.1f");
}