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

    void Visuals::DrawSkeletonESP() {
        if (!drawSkeleton) return;

        // Make sure memory system initialized
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;

        try {
            // Local player
            auto localPlayer = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwLocalPlayer);
            if (!localPlayer) return;
            int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);

            // Read view matrix from client.dll
            Matrix viewMatrix = Memory::Read<Matrix>(Memory::clientDll + offsets::dwViewMatrix);

            // Get ImGui display size for screen dimensions
            ImVec2 display = ImGui::GetIO().DisplaySize;
            int screenWidth = (int)display.x;
            int screenHeight = (int)display.y;

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            if (!drawList) return;

            // Loop through potential players
            for (int i = 1; i <= 64; ++i) {
                auto entity = Memory::Read<std::uintptr_t>(Memory::clientDll + offsets::dwEntityList + i * 0x10);
                if (!entity) continue;
                if (Memory::Read<bool>(entity + offsets::m_bDormant)) continue;
                if (entity == localPlayer) continue;

                int team = Memory::Read<int>(entity + offsets::m_iTeamNum);
                if (team == 0) continue;
                if (team == localTeam && teamCheck) continue;

                int health = Memory::Read<int>(entity + offsets::m_iHealth);
                if (health <= 0 || health > 100) continue;

                // Bone matrix
                uintptr_t boneMatrix = Memory::Read<uintptr_t>(entity + offsets::m_dwBoneMatrix);
                if (!boneMatrix) continue;

                // Bone vectors
                Vec3 pelvis, spine, chest, neck, head;
                Vec3 leftShoulder, leftElbow, leftHand;
                Vec3 rightShoulder, rightElbow, rightHand;
                Vec3 leftHip, leftKnee, leftFoot;
                Vec3 rightHip, rightKnee, rightFoot;

                // Read bones (corrected ValveBiped indices)
#define READ_BONE(vec, idx) \
                vec.x = Memory::Read<float>(boneMatrix + 0x30U * (idx) + 0x0C); \
                vec.y = Memory::Read<float>(boneMatrix + 0x30U * (idx) + 0x1C); \
                vec.z = Memory::Read<float>(boneMatrix + 0x30U * (idx) + 0x2C);

                READ_BONE(pelvis, 0);
                READ_BONE(spine, 5);
                READ_BONE(chest, 6);
                READ_BONE(neck, 7);
                READ_BONE(head, 8);

                READ_BONE(leftShoulder, 11);
                READ_BONE(leftElbow, 12);
                READ_BONE(leftHand, 13);

                READ_BONE(rightShoulder, 15);
                READ_BONE(rightElbow, 16);
                READ_BONE(rightHand, 17);

                READ_BONE(leftHip, 23);
                READ_BONE(leftKnee, 24);
                READ_BONE(leftFoot, 25);

                READ_BONE(rightHip, 26);
                READ_BONE(rightKnee, 27);
                READ_BONE(rightFoot, 28);

#undef READ_BONE

                // Project to screen
                Vec3 sPelvis, sSpine, sChest, sNeck, sHead;
                Vec3 sLShoulder, sLElbow, sLHand;
                Vec3 sRShoulder, sRElbow, sRHand;
                Vec3 sLHip, sLKnee, sLFoot;
                Vec3 sRHip, sRKnee, sRFoot;

                if (!WorldToScreen(pelvis, sPelvis, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(spine, sSpine, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(chest, sChest, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(neck, sNeck, viewMatrix, screenWidth, screenHeight) ||
                    !WorldToScreen(head, sHead, viewMatrix, screenWidth, screenHeight))
                    continue;

                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
                    skeletonColor[0], skeletonColor[1], skeletonColor[2], skeletonColor[3]));

                // Peito
                drawList->AddLine(ImVec2(sPelvis.x, sPelvis.y), ImVec2(sSpine.x, sSpine.y), col);
                drawList->AddLine(ImVec2(sSpine.x, sSpine.y), ImVec2(sChest.x, sChest.y), col);
                drawList->AddLine(ImVec2(sChest.x, sChest.y), ImVec2(sNeck.x, sNeck.y), col);
                drawList->AddLine(ImVec2(sNeck.x, sNeck.y), ImVec2(sHead.x, sHead.y), col);

                // Braço esquerdo
                if (WorldToScreen(leftShoulder, sLShoulder, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftElbow, sLElbow, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftHand, sLHand, viewMatrix, screenWidth, screenHeight)) {
                    drawList->AddLine(ImVec2(sChest.x, sChest.y), ImVec2(sLShoulder.x, sLShoulder.y), col);
                    drawList->AddLine(ImVec2(sLShoulder.x, sLShoulder.y), ImVec2(sLElbow.x, sLElbow.y), col);
                    drawList->AddLine(ImVec2(sLElbow.x, sLElbow.y), ImVec2(sLHand.x, sLHand.y), col);
                }

                // Braço direito 67
                if (WorldToScreen(rightShoulder, sRShoulder, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightElbow, sRElbow, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightHand, sRHand, viewMatrix, screenWidth, screenHeight)) {
                    drawList->AddLine(ImVec2(sChest.x, sChest.y), ImVec2(sRShoulder.x, sRShoulder.y), col);
                    drawList->AddLine(ImVec2(sRShoulder.x, sRShoulder.y), ImVec2(sRElbow.x, sRElbow.y), col);
                    drawList->AddLine(ImVec2(sRElbow.x, sRElbow.y), ImVec2(sRHand.x, sRHand.y), col);
                }

                // Perna esquerda
                if (WorldToScreen(leftHip, sLHip, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftKnee, sLKnee, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(leftFoot, sLFoot, viewMatrix, screenWidth, screenHeight)) {
                    drawList->AddLine(ImVec2(sPelvis.x, sPelvis.y), ImVec2(sLHip.x, sLHip.y), col);
                    drawList->AddLine(ImVec2(sLHip.x, sLHip.y), ImVec2(sLKnee.x, sLKnee.y), col);
                    drawList->AddLine(ImVec2(sLKnee.x, sLKnee.y), ImVec2(sLFoot.x, sLFoot.y), col);
                }

                // Perna direita
                if (WorldToScreen(rightHip, sRHip, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightKnee, sRKnee, viewMatrix, screenWidth, screenHeight) &&
                    WorldToScreen(rightFoot, sRFoot, viewMatrix, screenWidth, screenHeight)) {
                    drawList->AddLine(ImVec2(sPelvis.x, sPelvis.y), ImVec2(sRHip.x, sRHip.y), col);
                    drawList->AddLine(ImVec2(sRHip.x, sRHip.y), ImVec2(sRKnee.x, sRKnee.y), col);
                    drawList->AddLine(ImVec2(sRKnee.x, sRKnee.y), ImVec2(sRFoot.x, sRFoot.y), col);
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