// Visuals.cpp
#include "Visuals.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <windows.h>
#include "../../Gui/Imgui/imgui.h"
#include <cstdio>
#include <algorithm>

namespace Visuals {

    bool glowEnabled = true;
    bool glowThroughWalls = false;
    float glowColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    bool drawHealthBar = true;
    // | BOX ESP NEW BY DANTEZ |
    bool drawBoxes = true;
    bool teamCheck = true;
    float boxColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // default box color 
    // ------------------------------------------------------------------
    struct Vec3 { float x, y, z; };
    struct Matrix { float m[4][4]; };

    bool WorldToScreen(const Vec3& pos, Vec3& screen, const Matrix& vm, int width, int height) {
        float clipX = pos.x * vm.m[0][0] + pos.y * vm.m[0][1] + pos.z * vm.m[0][2] + vm.m[0][3];
        float clipY = pos.x * vm.m[1][0] + pos.y * vm.m[1][1] + pos.z * vm.m[1][2] + vm.m[1][3];
        float clipZ = pos.x * vm.m[2][0] + pos.y * vm.m[2][1] + pos.z * vm.m[2][2] + vm.m[2][3];
        float clipW = pos.x * vm.m[3][0] + pos.y * vm.m[3][1] + pos.z * vm.m[3][2] + vm.m[3][3];

        if (clipW < 0.01f) return false; 

        float invW = 1.0f / clipW;
        screen.x = (width / 2.0f) + (clipX * invW) * (width / 2.0f);
        screen.y = (height / 2.0f) - (clipY * invW) * (height / 2.0f);
        screen.z = clipZ * invW;
        return true;
    }

    void Glow() {
        if (!glowEnabled) return;
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;

        try {
            auto glowManager = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwGlowObjectManager);
            auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!glowManager || !localPlayer) return;

            int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
            printf("[Glow] localTeam: %d\n", localTeam);

            for (int i = 1; i < 32; i++) {  // Start from 1 to skip world
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity || Memory::Read<bool>(entity + offsets::m_bDormant)) {
                    // printf("[Glow] entity %d invalid\n", i);  // Comment out for less spam
                    continue;
                }

                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (team == localTeam || team == 0) continue;

                int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
                if (glowIndex < 0) continue;

                auto glowObject = glowManager + glowIndex * 0x38;
                Memory::Write<float>(glowObject + 0x8, glowColor[0]);
                Memory::Write<float>(glowObject + 0xC, glowColor[1]);
                Memory::Write<float>(glowObject + 0x10, glowColor[2]);
                Memory::Write<float>(glowObject + 0x14, glowColor[3]);
                Memory::Write<bool>(glowObject + 0x28, glowThroughWalls);
                Memory::Write<bool>(glowObject + 0x29, true);
            }

        }
        catch (...) { printf("[Glow] exception\n"); }
    }

    void DrawBoxESP() {
        if (!drawBoxes) return;  
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;

        try {
            
            auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) return;
            int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
            
            Matrix viewMatrix = Memory::Read<Matrix>(Memory::clientDll + offsets::dwViewMatrix);
            HWND hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
            RECT rect;
            if (hwnd) {
                GetClientRect(hwnd, &rect);
            }
            else {
                GetWindowRect(GetDesktopWindow(), &rect);
            }
            int screenWidth = rect.right - rect.left;
            int screenHeight = rect.bottom - rect.top;

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();  
           
            for (int i = 1; i < 32; ++i) {
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity) continue;
                if (Memory::Read<bool>(entity + offsets::m_bDormant)) continue;
                if (entity == localPlayer) continue; 
                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (team == 0) continue;              
                if (team == localTeam && teamCheck) continue;  

                int health = Memory::Read<int>(entity + offsets::m_iHealth);
                if (health <= 0 || health > 100) continue;    
               
                Vec3 origin = Memory::Read<Vec3>(entity + offsets::m_vecOrigin);
                Vec3 head = origin;
                head.z += 70.0f;  
               
                Vec3 screenFeet, screenHead;
                if (!WorldToScreen(origin, screenFeet, viewMatrix, screenWidth, screenHeight)) continue;
                if (!WorldToScreen(head, screenHead, viewMatrix, screenWidth, screenHeight)) continue;
               
                float top = screenHead.y;
                float bottom = screenFeet.y;
                if (top > bottom) std::swap(top, bottom);  // swap if needed so top is higher on screen:contentReference[oaicite:6]{index=6}

                float height = bottom - top;
                float width = height * 0.5f;              
                float halfWidth = width / 2.0f;
                float centerX = (screenHead.x + screenFeet.x) / 2.0f; 
                float left = centerX - halfWidth;
                float right = centerX + halfWidth;
               
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
                    boxColor[0], boxColor[1], boxColor[2], boxColor[3]));
                ImU32 outlineCol = IM_COL32(0, 0, 0, 255);  // black outline color

                // Draw box outline (black border) and main box
                drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), outlineCol, 0.0f, 0, 2.0f);
                drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), col, 0.0f, 0, 1.0f);

                // Debug output (optional): 
                printf("[BoxESP] Entity %d: left=%.1f, top=%.1f, right=%.1f, bottom=%.1f\n",
                    i, left, top, right, bottom);
            }
        }
        catch (...) {
            printf("[BoxESP] exception\n");
        }
    }

    void DrawHealthESP() {
        if (!drawHealthBar) return;
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;

        try {
            auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) return;

            int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);

            Matrix viewMatrix = Memory::Read<Matrix>(Memory::clientDll + offsets::dwViewMatrix);

            // Get game window dimensions for CS:GO
            HWND hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
            RECT rect;
            if (hwnd) {
                GetClientRect(hwnd, &rect);
            }
            else {
                GetWindowRect(GetDesktopWindow(), &rect);
            }
            int screenWidth = rect.right - rect.left;
            int screenHeight = rect.bottom - rect.top;

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            for (int i = 1; i < 32; i++) {
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity || Memory::Read<bool>(entity + offsets::m_bDormant)) continue;

                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (team == localTeam || team == 0) continue;

                int health = Memory::Read<int>(entity + offsets::m_iHealth);
                if (health <= 0 || health > 100) continue;

                Vec3 origin = Memory::Read<Vec3>(entity + offsets::m_vecOrigin);
                Vec3 head = origin; head.z += 70.f;

                Vec3 screenFeet, screenHead;
                if (!WorldToScreen(origin, screenFeet, viewMatrix, screenWidth, screenHeight)) continue;
                if (!WorldToScreen(head, screenHead, viewMatrix, screenWidth, screenHeight)) continue;

                float top = screenHead.y;
                float bottom = screenFeet.y;
                if (top > bottom) std::swap(top, bottom);

                float playerX = (screenHead.x + screenFeet.x) / 2.0f;
                float barX = playerX - 15.0f;
                float barHalfWidth = 3.0f;

                // drawList->AddLine(ImVec2(screenWidth / 2.0f, screenHeight / 2.0f), ImVec2(screenFeet.x, screenFeet.y), IM_COL32(255, 255, 255, 255), 1.0f);

                ImU32 borderColor = IM_COL32(0, 0, 0, 255); // borda preta para contraste
                drawList->AddRect(ImVec2(barX - barHalfWidth, top), ImVec2(barX + barHalfWidth, bottom), borderColor, 0.0f, 0, 1.0f);

                ImU32 bgColor = IM_COL32(50, 50, 50, 180);
                drawList->AddRectFilled(ImVec2(barX - barHalfWidth, top), ImVec2(barX + barHalfWidth, bottom), bgColor);

                float height = bottom - top;
                if (height <= 0) continue;
                float filled = (static_cast<float>(health) / 100.0f) * height;
                ImU32 fillColor = IM_COL32(0, 255, 0, 255); // Verde fixo
                drawList->AddRectFilled(ImVec2(barX - barHalfWidth, bottom - filled), ImVec2(barX + barHalfWidth, bottom), fillColor);

                printf("[HealthESP] Entity %d: health=%d, top=%.1f, bottom=%.1f, filled=%.1f\n", i, health, top, bottom, filled);

            }

        }
        catch (...) { printf("[HealthESP] exception\n"); }
    }

}