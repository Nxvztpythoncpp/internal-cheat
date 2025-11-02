#include "Aimbot.h"
#include "../../Memory/Memory.h"
#include <algorithm>
#include "../../Memory/Offsets.h"
#include <cmath>
#include <cstdint>
#include <Windows.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Aimbot {
    bool enabled = false;
    float fov = 180.0f;
    bool draw_fov_circle = false;
    bool silent = false;
    bool rcs = false;
    bool visibility_check = false;
    bool team_check = false;
    int aim_key = VK_XBUTTON2;
    int aim_bone = 8;
    float smooth_factor = 1.0f;



    bool IsKeyPressed(int key) {
        bool pressed = (GetAsyncKeyState(key) & 0x8000) != 0;
        // Log key state occasionally could be noisy, so only log when pressed
        if (pressed) {
            //printf("[Aimbot] Key %d pressed\n", key);
        }
        return pressed;
    }

    static uintptr_t GetClientStatePtr() {
        uintptr_t clientStatePtr = Memory::Read<uintptr_t>((uintptr_t)Memory::engineDll + offsets::dwClientState);
        //printf("[Aimbot] GetClientStatePtr -> %p\n", (void*)clientStatePtr);
        return clientStatePtr;
    }

    void ApplyRecoilControl(uintptr_t localPlayer, QAngle& angles) {
        //printf("[Aimbot] ApplyRecoilControl called. rcs=%d\n", (int)rcs);
        if (!rcs) {
            //printf("[Aimbot] RCS disabled, skipping.\n");
            return;
        }

        int localShotsFired = Memory::Read<int>(localPlayer + offsets::m_iShotsFired);
        //printf("[Aimbot] localShotsFired = %d\n", localShotsFired);
        if (localShotsFired <= 1) {
            //printf("[Aimbot] Not enough shots fired, skipping RCS.\n");
            return;
        }

        Vector aimPunch = Memory::Read<Vector>(localPlayer + offsets::m_aimPunchAngle);
        QAngle aimPunchAngle = VectorToQAngle(aimPunch);

        //printf("[Aimbot] aimPunchAngle x=%f y=%f z=%f\n", aimPunchAngle.x, aimPunchAngle.y, aimPunchAngle.z);

        angles = angles - (aimPunchAngle * 2.0f);
        ClampAngles(angles);
        //printf("[Aimbot] Angles after RCS x=%f y=%f z=%f\n", angles.x, angles.y, angles.z);
    }

    void Run() {
        //printf("[Aimbot] Run() entry. enabled=%d silent=%d aim_key=%d\n", (int)enabled, (int)silent, aim_key);
        if (Memory::clientDll == 0 && !Memory::Initialize()) return;
        OutputDebugStringA(" Aimbot::Run() trigger\n");

        if (!enabled) {
            //printf("[Aimbot] Aimbot disabled, returning.\n");
            return;
        }
        if (aim_key != 0 && !IsKeyPressed(aim_key)) {
            //printf("[Aimbot] Aim key not pressed, returning.\n");
            return;
        }

        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        //printf("[Aimbot] localPlayer ptr = %p\n", (void*)localPlayer);
        if (!localPlayer) {
            //printf("[Aimbot] No local player found, returning.\n");
            return;
        }

        uintptr_t bestTarget = GetBestTarget(fov);
        //printf("[Aimbot] bestTarget = %p\n", (void*)bestTarget);
        if (bestTarget == 0) {
            //printf("[Aimbot] No target found within fov=%f, returning.\n", fov);
            return;
        }

        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);
        //printf("[Aimbot] localEyePos x=%f y=%f z=%f\n", localEyePos.x, localEyePos.y, localEyePos.z);

        Vector targetBone = GetBonePosition(bestTarget, aim_bone);
        //printf("[Aimbot] targetBone x=%f y=%f z=%f\n", targetBone.x, targetBone.y, targetBone.z);

        QAngle aimAngle = CalculateAngle(localEyePos, targetBone);
        //printf("[Aimbot] raw aimAngle x=%f y=%f z=%f\n", aimAngle.x, aimAngle.y, aimAngle.z);

        ApplyRecoilControl(localPlayer, aimAngle);
        ClampAngles(aimAngle);
        //printf("[Aimbot] aimAngle after RCS+Clamp x=%f y=%f z=%f\n", aimAngle.x, aimAngle.y, aimAngle.z);

        uintptr_t clientState = GetClientStatePtr();
        if (!clientState) {
            //printf("[Aimbot] clientState not found, returning.\n");
            return;
        }

        Vector currentAnglesVec = Memory::Read<Vector>(clientState + offsets::dwClientState_ViewAngles);
        QAngle currentAngles = VectorToQAngle(currentAnglesVec);
        //printf("[Aimbot] current view angles x=%f y=%f z=%f\n", currentAngles.x, currentAngles.y, currentAngles.z);

        // Smooth the aimAngle towards the current view
        SmoothAngle(currentAngles, aimAngle, smooth_factor);
        //printf("[Aimbot] aimAngle after Smooth x=%f y=%f z=%f (smooth_factor=%f)\n", aimAngle.x, aimAngle.y, aimAngle.z, smooth_factor);

        if (!silent) {
            // Write the (possivelmente) suavizado aimAngle para viewangles do clientState
            Memory::Write<Vector>(clientState + offsets::dwClientState_ViewAngles, QAngleToVector(aimAngle));
            //printf("[Aimbot] Wrote viewangles to clientState: x=%f y=%f z=%f\n", aimAngle.x, aimAngle.y, aimAngle.z);
        }
        else {
            //printf("[Aimbot] Silent aim enabled, not writing viewangles.\n");
        }
    }

    void OnCreateMove() {

        OutputDebugStringA("OnCreateMove() triggered\n");
        //printf("[Aimbot] OnCreateMove() entry. enabled=%d silent=%d\n", (int)enabled, (int)silent);

        if (!enabled || !silent) {
            //printf("[Aimbot] OnCreateMove skipped (either disabled or not silent mode).\n");
            return;
        }
        if (aim_key != 0 && !IsKeyPressed(aim_key)) {
            //printf("[Aimbot] Aim key not pressed in OnCreateMove, returning.\n");
            return;
        }

        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        //printf("[Aimbot] (OnCreateMove) localPlayer ptr = %p\n", (void*)localPlayer);
        if (!localPlayer) {
            //printf("[Aimbot] (OnCreateMove) No local player found, returning.\n");
            return;
        }

        uintptr_t bestTarget = GetBestTarget(fov);
        //printf("[Aimbot] (OnCreateMove) bestTarget = %p\n", (void*)bestTarget);
        if (bestTarget == 0) {
            //printf("[Aimbot] (OnCreateMove) No target found within fov=%f, returning.\n", fov);
            return;
        }

        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);
        //printf("[Aimbot] (OnCreateMove) localEyePos x=%f y=%f z=%f\n", localEyePos.x, localEyePos.y, localEyePos.z);

        Vector targetBone = GetBonePosition(bestTarget, aim_bone);
        //printf("[Aimbot] (OnCreateMove) targetBone x=%f y=%f z=%f\n", targetBone.x, targetBone.y, targetBone.z);

        QAngle aimAngle = CalculateAngle(localEyePos, targetBone);
        //printf("[Aimbot] (OnCreateMove) raw aimAngle x=%f y=%f z=%f\n", aimAngle.x, aimAngle.y, aimAngle.z);

        ApplyRecoilControl(localPlayer, aimAngle);
        ClampAngles(aimAngle);
        //printf("[Aimbot] (OnCreateMove) aimAngle after RCS+Clamp x=%f y=%f z=%f\n", aimAngle.x, aimAngle.y, aimAngle.z);


    }

    QAngle CalculateAngle(const Vector& source, const Vector& destination) {
        Vector delta = destination - source;
        QAngle angles;
        angles.x = -std::atan2(delta.z, std::hypot(delta.x, delta.y)) * (180.0f / M_PI);
        angles.y = std::atan2(delta.y, delta.x) * (180.0f / M_PI);
        angles.z = 0.0f;
        // Log calculated angle
        /*//printf("[Aimbot] CalculateAngle -> x=%f y=%f z=%f (delta x=%f y=%f z=%f)\n",
            angles.x, angles.y, angles.z, delta.x, delta.y, delta.z);*/
        return angles;
    }

    void ClampAngles(QAngle& angles) {
        if (angles.x > 89.0f) angles.x = 89.0f;
        if (angles.x < -89.0f) angles.x = -89.0f;
        while (angles.y > 180.0f) angles.y -= 360.0f;
        while (angles.y < -180.0f) angles.y += 360.0f;
        angles.z = 0.0f;
        // Optionally log when clamping happens (can spam) - print only final values
        //printf("[Aimbot] ClampAngles result x=%f y=%f z=%f\n", angles.x, angles.y, angles.z);
    }

    void SmoothAngle(const QAngle& current, QAngle& target, float smoothFactor) {
        /*//printf("[Aimbot] SmoothAngle called. current x=%f y=%f z=%f target-before x=%f y=%f z=%f factor=%f\n",
            current.x, current.y, current.z, target.x, target.y, target.z, smoothFactor);*/
        if (smoothFactor <= 1.0f) {
            //printf("[Aimbot] smoothFactor <= 1, skipping smoothing.\n");
            return;
        }
        QAngle delta = target - current;
        ClampAngles(delta);
        target = current + delta / smoothFactor;
        ClampAngles(target);
        //printf("[Aimbot] SmoothAngle result target-after x=%f y=%f z=%f\n", target.x, target.y, target.z);
    }

    uintptr_t GetBestTarget(float maxFov) {
        //printf("[Aimbot] GetBestTarget called with maxFov=%f\n", maxFov);
        uintptr_t localPlayer = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwLocalPlayer);
        //printf("[Aimbot] (GetBestTarget) localPlayer ptr = %p\n", (void*)localPlayer);
        if (!localPlayer) {
            //printf("[Aimbot] (GetBestTarget) No local player, returning 0.\n");
            return 0;
        }

        int localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
        Vector localEyePos = Memory::Read<Vector>(localPlayer + offsets::m_vecOrigin) +
            Memory::Read<Vector>(localPlayer + offsets::m_vecViewOffset);
        //printf("[Aimbot] localTeam=%d localEyePos x=%f y=%f z=%f\n", localTeam, localEyePos.x, localEyePos.y, localEyePos.z);

        uintptr_t clientState = GetClientStatePtr();
        if (!clientState) {
            //printf("[Aimbot] No clientState, returning 0.\n");
            return 0;
        }

        Vector viewAnglesVec = Memory::Read<Vector>(clientState + offsets::dwClientState_ViewAngles);
        QAngle viewAngles = VectorToQAngle(viewAnglesVec);
        //printf("[Aimbot] viewAngles x=%f y=%f z=%f\n", viewAngles.x, viewAngles.y, viewAngles.z);

        float bestFov = maxFov;
        uintptr_t bestTarget = 0;

        for (int i = 1; i < 32; i++) {
            uintptr_t entity = Memory::Read<uintptr_t>((uintptr_t)Memory::clientDll + offsets::dwEntityList + i * 0x10);
            if (!entity) {
                ////printf("[Aimbot] Entity %d is null, skipping.\n", i);
                continue;
            }
            // Print entity pointer for tracing
            //printf("[Aimbot] Checking entity %d -> %p\n", i, (void*)entity);

            if (!IsValidTarget(entity)) {
                //printf("[Aimbot] Entity %d failed IsValidTarget\n", i);
                continue;
            }

            int entityTeam = Memory::Read<int>(entity + offsets::m_iTeamNum);
            if (team_check && entityTeam == localTeam) {
                //printf("[Aimbot] Entity %d is same team (%d), skipping due to team_check\n", i, entityTeam);
                continue;
            }
            if (visibility_check && !IsVisible(entity)) {
                //printf("[Aimbot] Entity %d not visible, skipping due to visibility_check\n", i);
                continue;
            }

            Vector entityBone = GetBonePosition(entity, aim_bone);
            //printf("[Aimbot] Entity %d bone pos x=%f y=%f z=%f\n", i, entityBone.x, entityBone.y, entityBone.z);

            QAngle angleToTarget = CalculateAngle(localEyePos, entityBone);

            float deltaX = angleToTarget.x - viewAngles.x;
            float deltaY = angleToTarget.y - viewAngles.y;

            float currentFov = std::sqrt(deltaX * deltaX + deltaY * deltaY);

            // Log candidate fov
            //printf("[Aimbot] Entity %d fov=%f (bestFov=%f)\n", i, currentFov, bestFov);

            if (currentFov < bestFov) {
                bestFov = currentFov;
                bestTarget = entity;
                //printf("[Aimbot] New best target: entity %d (%p) with fov=%f\n", i, (void*)entity, currentFov);
            }
        }

        if (bestTarget) {
            //printf("[Aimbot] GetBestTarget -> %p (bestFov=%f)\n", (void*)bestTarget, bestFov);
        }
        else {
            //printf("[Aimbot] GetBestTarget found no target within fov=%f\n", maxFov);
        }

        return bestTarget;
    }

    bool IsValidTarget(uintptr_t entity) {
        if (!entity) {
            //printf("[Aimbot] IsValidTarget called with null entity\n");
            return false;
        }
        int health = Memory::Read<int>(entity + offsets::m_iHealth);
        if (health <= 0 || health > 100) {
            //printf("[Aimbot] Entity %p invalid health=%d\n", (void*)entity, health);
            return false;
        }
        bool dormant = Memory::Read<bool>(entity + offsets::m_bDormant);
        if (dormant) {
            //printf("[Aimbot] Entity %p is dormant\n", (void*)entity);
        }
        return !dormant;
    }

    bool IsVisible(uintptr_t entity) {
        if (!visibility_check) {
            //printf("[Aimbot] visibility_check disabled -> treating entity as visible\n");
            return true;
        }
        if (!entity) {
            //printf("[Aimbot] IsVisible called with null entity\n");
            return false;
        }

        uintptr_t clientState = GetClientStatePtr();
        if (!clientState) {
            //printf("[Aimbot] IsVisible: clientState not found\n");
            return false;
        }

        int localIndex = Memory::Read<int>(clientState + offsets::dwClientState_GetLocalPlayer);
        if (localIndex <= 0) {
            //printf("[Aimbot] IsVisible: invalid localIndex=%d\n", localIndex);
            return false;
        }

        uint32_t spottedByMask = Memory::Read<uint32_t>(entity + offsets::m_bSpottedByMask);
        bool visible = (spottedByMask & (1 << (localIndex - 1))) != 0;
        //printf("[Aimbot] IsVisible entity=%p spottedByMask=0x%X localIndex=%d visible=%d\n", (void*)entity, spottedByMask, localIndex, (int)visible);
        return visible;
    }

    Vector GetBonePosition(uintptr_t entity, int boneIndex) {
        Vector bonePos = { 0.0f, 0.0f, 0.0f };
        if (!entity) {
            //printf("[Aimbot] GetBonePosition called with null entity\n");
            return bonePos;
        }

        uintptr_t boneMatrix = Memory::Read<uintptr_t>(entity + offsets::m_dwBoneMatrix);
        //printf("[Aimbot] GetBonePosition entity=%p boneMatrix=%p boneIndex=%d\n", (void*)entity, (void*)boneMatrix, boneIndex);
        if (!boneMatrix) {
            //printf("[Aimbot] Bone matrix null for entity=%p\n", (void*)entity);
            return bonePos;
        }

        bonePos.x = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x0C);
        bonePos.y = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x1C);
        bonePos.z = Memory::Read<float>(boneMatrix + 0x30 * boneIndex + 0x2C);
        //printf("[Aimbot] BonePos computed x=%f y=%f z=%f\n", bonePos.x, bonePos.y, bonePos.z);
        return bonePos;
    }
}
