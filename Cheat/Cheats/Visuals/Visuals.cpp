// Visuals.cpp
#include "Visuals.h"
#include "../../Memory/Memory.h"
#include "../../Memory/Offsets.h"
#include <windows.h>
#include "../../Gui/Imgui/imgui.h"
#include <cstdio>
#include <algorithm>

namespace Visuals {

    bool espEnabled = false;
    bool glowEnabled = false;
    bool glowThroughWalls = false;
    float glowColorTeam[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    float glowColorEnemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    bool drawHealthBar = false;
    // | BOX ESP NEW BY DANTEZ |
    bool drawBoxes = false;
    bool teamCheck = false;
    bool boxOutline = false;
    float boxColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // default box color 
    bool drawSkeleton = false;
	float skeletonColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // default skeleton color like in box esp (red is my fav)
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
            //printf("[Glow] localTeam: %d\n", localTeam);

            for (int i = 1; i < 32; i++) {  // Start from 1 to skip world
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity || Memory::Read<bool>(entity + offsets::m_bDormant)) {
                    // printf("[Glow] entity %d invalid\n", i);  // Comment out for less spam
                    continue;
                }

                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (!teamCheck) {
                    if (team == localTeam) {
                        int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
                        if (glowIndex < 0) continue;

                        auto glowObject = glowManager + glowIndex * 0x38;
                        Memory::Write<float>(glowObject + 0x8, glowColorTeam[0]);
                        Memory::Write<float>(glowObject + 0xC, glowColorTeam[1]);
                        Memory::Write<float>(glowObject + 0x10, glowColorTeam[2]);
                        Memory::Write<float>(glowObject + 0x14, glowColorTeam[3]);
                        Memory::Write<bool>(glowObject + 0x28, glowThroughWalls);
                    }
                    else {
                        if (team != localTeam) {
                            int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
                            if (glowIndex < 0) continue;

                            auto glowObject = glowManager + glowIndex * 0x38;
                            Memory::Write<float>(glowObject + 0x8, glowColorEnemy[0]);
                            Memory::Write<float>(glowObject + 0xC, glowColorEnemy[1]);
                            Memory::Write<float>(glowObject + 0x10, glowColorEnemy[2]);
                            Memory::Write<float>(glowObject + 0x14, glowColorEnemy[3]);
                            Memory::Write<bool>(glowObject + 0x28, glowThroughWalls);
                        }
                    }

                }
                else {
                    if (team == localTeam || team == 0) continue;

                    int glowIndex = Memory::Read<int>(entity + offsets::m_iGlowIndex);
                    if (glowIndex < 0) continue;

                    auto glowObject = glowManager + glowIndex * 0x38;
                    Memory::Write<float>(glowObject + 0x8, glowColorEnemy[0]);
                    Memory::Write<float>(glowObject + 0xC, glowColorEnemy[1]);
                    Memory::Write<float>(glowObject + 0x10, glowColorEnemy[2]);
                    Memory::Write<float>(glowObject + 0x14, glowColorEnemy[3]);
                    Memory::Write<bool>(glowObject + 0x28, glowThroughWalls);
                    //Memory::Write<bool>(glowObject + 0x29, !glowThroughWalls);
                }
            }

        }
        catch (...) { printf("[Glow] exception\n"); }
    }

    void DrawSkeletonESP() {
        if (!drawSkeleton) return;
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;

        try {
            auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) return;
            int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);

            // Read view matrix and get screen dimensions
            Matrix viewMatrix = Memory::Read<Matrix>(Memory::clientDll + offsets::dwViewMatrix);
            HWND hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
            RECT rect;
            if (hwnd) GetClientRect(hwnd, &rect);
            else GetWindowRect(GetDesktopWindow(), &rect);
            int screenWidth = rect.right - rect.left;
            int screenHeight = rect.bottom - rect.top;

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            // Loop through entity list
            for (int i = 1; i < 32; ++i) {
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity) continue;
                if (Memory::Read<bool>(entity + offsets::m_bDormant)) continue;
                if (entity == localPlayer) continue;
                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (team == 0) continue;
                if (team == localTeam && teamCheck) continue;  // respect team check
                int health = Memory::Read<int>(entity + offsets::m_iHealth);
                if (health <= 0 || health > 100) continue;

                // Read key bones (using the bone matrix)
                uintptr_t boneMatrix = Memory::Read<uintptr_t>(entity + offsets::m_dwBoneMatrix);
                if (!boneMatrix) continue;
                Vec3 pelvis, spine, chest, neck, head;
                Vec3 leftShoulder, leftElbow, leftHand;
                Vec3 rightShoulder, rightElbow, rightHand;
                Vec3 leftHip, leftKnee, leftFoot;
                Vec3 rightHip, rightKnee, rightFoot;

                // Helper macro to read each bone's (x,y,z)
                #define READ_BONE(vec, index) \
                    vec.x = Memory::Read<float>(boneMatrix + 0x30 * (index) + 0x0C); \
                    vec.y = Memory::Read<float>(boneMatrix + 0x30 * (index) + 0x1C); \
                    vec.z = Memory::Read<float>(boneMatrix + 0x30 * (index) + 0x2C);

                READ_BONE(pelvis, 0);
                READ_BONE(spine, 2);
                READ_BONE(chest, 3);
                READ_BONE(neck, 4);
                READ_BONE(head, 5);

                READ_BONE(leftShoulder, 6);
                READ_BONE(leftElbow, 7);
                READ_BONE(leftHand, 8);

                READ_BONE(rightShoulder, 9);
                READ_BONE(rightElbow, 10);
                READ_BONE(rightHand, 11);

                READ_BONE(leftHip, 12);
                READ_BONE(leftKnee, 13);
                READ_BONE(leftFoot, 14);

                READ_BONE(rightHip, 15);
                READ_BONE(rightKnee, 16);
                READ_BONE(rightFoot, 17);
                #undef READ_BONE

                // Project bones to screen
                Vec3 s0, s2, s3, s4, s5, s6;
                Vec3 s8, s9, s10, s13, s14, s15;
                Vec3 s17, s18, s19, s20, s22, s23, s24, s25;
                if (!WorldToScreen(pelvis, s0, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(spine, s2, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(chest, s3, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(neck, s4, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(head, s5, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(leftShoulder, s6, viewMatrix, screenWidth, screenHeight))
                    continue; // need at least torso/head visible

                // Compute color
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
                    skeletonColor[0], skeletonColor[1], skeletonColor[2], skeletonColor[3]));

                // Draw spine (pelvis->stomach->chest->neck->head)
                drawList->AddLine(ImVec2(s0.x, s0.y), ImVec2(s2.x, s2.y), col);
                drawList->AddLine(ImVec2(s2.x, s2.y), ImVec2(s3.x, s3.y), col);
                drawList->AddLine(ImVec2(s3.x, s3.y), ImVec2(s4.x, s4.y), col);
                drawList->AddLine(ImVec2(s4.x, s4.y), ImVec2(s5.x, s5.y), col);
                drawList->AddLine(ImVec2(s5.x, s5.y), ImVec2(s6.x, s6.y), col);

                // Draw left arm (neck->shoulder->elbow->hand)
                if (WorldToScreen(leftHand, s8, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightShoulder, s9, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightElbow, s10, viewMatrix, screenWidth, screenHeight))
                {
                    drawList->AddLine(ImVec2(s5.x, s5.y), ImVec2(s8.x, s8.y), col);
                    drawList->AddLine(ImVec2(s8.x, s8.y), ImVec2(s9.x, s9.y), col);
                    drawList->AddLine(ImVec2(s9.x, s9.y), ImVec2(s10.x, s10.y), col);
                }

                // Draw right arm (neck->shoulder->elbow->hand)
                if (WorldToScreen(rightShoulder, s13, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightElbow, s14, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightHand, s15, viewMatrix, screenWidth, screenHeight))
                {
                    drawList->AddLine(ImVec2(s5.x, s5.y), ImVec2(s13.x, s13.y), col);
                    drawList->AddLine(ImVec2(s13.x, s13.y), ImVec2(s14.x, s14.y), col);
                    drawList->AddLine(ImVec2(s14.x, s14.y), ImVec2(s15.x, s15.y), col);
                }

                // Draw left leg (pelvis->thigh->knee->foot)
                if (WorldToScreen(leftHip, s22, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftKnee, s23, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftFoot, s25, viewMatrix, screenWidth, screenHeight))
                {
                    drawList->AddLine(ImVec2(s0.x, s0.y), ImVec2(s22.x, s22.y), col);
                    drawList->AddLine(ImVec2(s22.x, s22.y), ImVec2(s23.x, s23.y), col);
                    drawList->AddLine(ImVec2(s23.x, s23.y), ImVec2(s25.x, s25.y), col);
                }

                // Draw right leg (pelvis->thigh->knee->foot)
                if (WorldToScreen(rightHip, s17, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightKnee, s18, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightFoot, s20, viewMatrix, screenWidth, screenHeight))
                {
                    drawList->AddLine(ImVec2(s0.x, s0.y), ImVec2(s17.x, s17.y), col);
                    drawList->AddLine(ImVec2(s17.x, s17.y), ImVec2(s18.x, s18.y), col);
                    drawList->AddLine(ImVec2(s18.x, s18.y), ImVec2(s20.x, s20.y), col);
                }
            }
        }
        catch (...) {
            printf("[SkeletonESP] exception\n");
        }
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

                if (boxOutline) { drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), outlineCol, 0.0f, 0, 2.0f); };
                drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), col, 0.0f, 0, 1.0f);

                /*printf("[BoxESP] Entity %d: left=%.1f, top=%.1f, right=%.1f, bottom=%.1f\n",
                    i, left, top, right, bottom);*/
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

            HWND hwnd = FindWindowA(NULL, "Counter-Strike: Global Offensive");
            RECT rect;
            if (hwnd) GetClientRect(hwnd, &rect);
            else GetWindowRect(GetDesktopWindow(), &rect);

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
                Vec3 head = origin; head.z += 72.f;

                Vec3 screenFeet, screenHead;
                if (!WorldToScreen(origin, screenFeet, viewMatrix, screenWidth, screenHeight)) continue;
                if (!WorldToScreen(head, screenHead, viewMatrix, screenWidth, screenHeight)) continue;

                float top = screenHead.y;
                float bottom = screenFeet.y;
                if (top > bottom) std::swap(top, bottom);

                float height = bottom - top;
                float width = height * 0.5f;
                float halfWidth = width / 2.0f;
                float centerX = (screenHead.x + screenFeet.x) / 2.0f;

                float barWidth = 4.0f;
                float barX;

                if (drawBoxes) {
                    float left = centerX - halfWidth;
                    barX = left - 6.0f - barWidth;
                }
                else {
                    float left = centerX - halfWidth;
                    barX = left - 5.0f - barWidth;
                }

                ImU32 borderColor = IM_COL32(0, 0, 0, 255);
                ImU32 bgColor = IM_COL32(50, 50, 50, 180);
                drawList->AddRectFilled(ImVec2(barX, top), ImVec2(barX + barWidth, bottom), bgColor);
                drawList->AddRect(ImVec2(barX, top), ImVec2(barX + barWidth, bottom), borderColor);

                float filled = (static_cast<float>(health) / 100.0f) * (bottom - top);
                ImU32 fillColor = IM_COL32(0, 255, 0, 255);
                drawList->AddRectFilled(ImVec2(barX, bottom - filled), ImVec2(barX + barWidth, bottom), fillColor);

                char hpText[8]; sprintf_s(hpText, sizeof(hpText), "%d", health);
                drawList->AddText(ImVec2(barX - 15, bottom - filled - 10), IM_COL32(255, 255, 255, 255), hpText);
            }
        }
        catch (...) { printf("[HealthESP] exception\n"); }
    }



}