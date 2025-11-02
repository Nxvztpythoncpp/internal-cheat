#include "../Imgui/imgui.h"
#include "../../Cheats/Rage/Aimbot.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"

#define M_PI 3.14159265358979323846f

void DrawFovCircle() {
    if (!Aimbot::enabled || !Aimbot::draw_fov_circle) return;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

    // REAL pixel radius using viewmatrix projection
    float* vm = Memory::Read<float*>(Memory::clientDll + offsets::dwViewMatrix);
    if (!vm) return;

    float w = vm[11] * center.y + vm[15];
    if (w < 0.01f) return;

    float inv_w = 1.0f / w;
    float fov_rad = Aimbot::fov * M_PI / 360.0f;
    float radius = center.y * tanf(fov_rad) * inv_w;

    draw->AddCircle(center, radius, IM_COL32(180, 0, 255, 220), 64, 2.2f);
    draw->AddCircle(center, radius + 1, IM_COL32(255, 255, 255, 40), 64, 1.0f);
}