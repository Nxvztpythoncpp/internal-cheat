#include "Aimbot.h"
#include "../../Memory/Memory.h"
#include <algorithm>
#include "../../Memory/Offsets.h"
#include <cmath>
#include <cstdint>
#include <Windows.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Aimbot {
    bool enabled = false;
    float fov = 3.0f;
    bool silent = false;
    bool rcs = true;
    bool visibility_check = true;
    bool team_check = true;
    int aim_key = VK_XBUTTON2;
    int aim_bone = 8;
    float smooth_factor = 10.0f;

    bool IsKeyPressed(int key) {
        return (GetAsyncKeyState(key) & 0x8000) != 0;
    }

    static uintptr_t GetClientStatePtr() {
        uintptr_t clientStatePtr = Memory::Read<uintptr_t>((uintptr_t)Memory::engineDll + offsets::dwClientState);
        return clientStatePtr;
    }

    void ApplyRecoilControl(uintptr_t localPlayer, QAngle& angles) {
        if (!rcs) return;

        int localShotsFired = Memory::Read<int>(localPlayer + offsets::m_iShotsFired);
        if (localShotsFired <= 1) return;

        Vector aimPunch = Memory::Read<Vector>(localPlayer + offsets::m_aimPunchAngle);
        QAngle aimPunchAngle = VectorToQAngle(aimPunch);

        angles = angles - (aimPunchAngle * 2.0f);
        ClampAngles(angles);
    }

    void Run() {
        OutputDebugStringA(" Aimbot::Run() trigger\n");

        if (!enabled) return;
        if (aim_key != 0 && !IsKeyPressed(aim_key)) return;

        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return;

        int bestTarget = GetBestTarget(fov);
        if (bestTarget == -1) return;

        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);

        Vector targetBone = GetBonePosition(bestTarget, aim_bone);
        QAngle aimAngle = CalculateAngle(localEyePos, targetBone);

        ApplyRecoilControl(localPlayer, aimAngle);
        ClampAngles(aimAngle);

        uintptr_t clientState = GetClientStatePtr();
        if (!clientState) return;

        Vector currentAnglesVec = Memory::Read<Vector>(clientState + offsets::dwClientState_ViewAngles);
        QAngle currentAngles = VectorToQAngle(currentAnglesVec);
        SmoothAngle(currentAngles, aimAngle, smooth_factor);

        if (!silent) {
            Memory::Write<Vector>(clientState + offsets::dwClientState_ViewAngles, QAngleToVector(aimAngle));
        }
    }

    void OnCreateMove() {
        OutputDebugStringA("OnCreateMove() triggered\n");

        if (!enabled || !silent) return;
        if (aim_key != 0 && !IsKeyPressed(aim_key)) return;

        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return;

        int bestTarget = GetBestTarget(fov);
        if (bestTarget == -1) return;

        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);

        Vector targetBone = GetBonePosition(bestTarget, aim_bone);
        QAngle aimAngle = CalculateAngle(localEyePos, targetBone);

        ApplyRecoilControl(localPlayer, aimAngle);
        ClampAngles(aimAngle);
    }

    QAngle CalculateAngle(const Vector& source, const Vector& destination) {
        Vector delta = destination - source;
        QAngle angles;
        angles.x = -std::atan2(delta.z, std::hypot(delta.x, delta.y)) * (180.0f / M_PI);
        angles.y = std::atan2(delta.y, delta.x) * (180.0f / M_PI);
        angles.z = 0.0f;
        return angles;
    }

    void ClampAngles(QAngle& angles) {
        if (angles.x > 89.0f) angles.x = 89.0f;
        if (angles.x < -89.0f) angles.x = -89.0f;
        while (angles.y > 180.0f) angles.y -= 360.0f;
        while (angles.y < -180.0f) angles.y += 360.0f;
        angles.z = 0.0f;
    }

    void SmoothAngle(const QAngle& current, QAngle& target, float smoothFactor) {
        if (smoothFactor <= 1.0f) return;
        QAngle delta = target - current;
        ClampAngles(delta);
        target = current + delta / smoothFactor;
        ClampAngles(target);
    }

    int GetBestTarget(float maxFov) {
        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return -1;

        int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);

        uintptr_t clientState = GetClientStatePtr();
        if (!clientState) return -1;

        Vector viewAnglesVec = Memory::Read<Vector>(clientState + offsets::dwClientState_ViewAngles);
        QAngle viewAngles = VectorToQAngle(viewAnglesVec);

        float bestFov = maxFov;
        int bestTarget = -1;

        for (int i = 1; i < 32; i++) {
            uintptr_t entity = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwEntityList + i * 0x10);
            if (!entity || !IsValidTarget((int)entity)) continue;

            int entityTeam = Memory::Read<int>(entity + offsets::m_iTeamNum);
            if (team_check && entityTeam == localTeam) continue;
            if (visibility_check && !IsVisible((int)entity)) continue;

            Vector entityBone = GetBonePosition((int)entity, aim_bone);
            QAngle angleToTarget = CalculateAngle(localEyePos, entityBone);

            float deltaX = angleToTarget.x - viewAngles.x;
            float deltaY = angleToTarget.y - viewAngles.y;
            float currentFov = std::sqrt(deltaX * deltaX + deltaY * deltaY);

            if (currentFov < bestFov) {
                bestFov = currentFov;
                bestTarget = (int)entity;
            }
        }

        return bestTarget;
    }

    bool IsValidTarget(int entity) {
        if (!entity) return false;
        int health = Memory::Read<int>(entity + offsets::m_iHealth);
        if (health <= 0 || health > 100) return false;
        bool dormant = Memory::Read<bool>(entity + offsets::m_bDormant);
        return !dormant;
    }

    bool IsVisible(int entity) {
        if (!visibility_check) return true;
        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        if (!localPlayer) return false;
        int localIndex = Memory::Read<int>(localPlayer + 0x64);
        uint32_t spottedByMask = Memory::Read<uint32_t>(entity + offsets::m_bSpottedByMask);
        return (spottedByMask & (1 << (localIndex - 1))) != 0;
    }

    Vector GetBonePosition(int entity, int boneIndex) {
        uintptr_t boneMatrix = Memory::Read<uintptr_t>(entity + offsets::m_dwBoneMatrix);
        Vector bonePos;
        bonePos.x = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x0C);
        bonePos.y = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x1C);
        bonePos.z = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x2C);
        return bonePos;
    }
}