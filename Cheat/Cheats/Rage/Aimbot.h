#pragma once
#include "../../Memory/Offsets.h"
#include <Windows.h>
#include <cmath>
#include <cstdint>               // necessário para uintptr_t
#include "../../Dependencies/Hooks/Hooks.h"

namespace Aimbot {
    extern bool enabled;
    extern float fov;
    extern bool silent;
    extern bool rcs;
    extern bool visibility_check;
    extern bool team_check;
    extern int aim_key;
    extern int aim_bone;
    extern float smooth_factor;

    void Run();
    void OnCreateMove();

    QAngle CalculateAngle(const Vector& source, const Vector& destination);
    void ClampAngles(QAngle& angles);
    void SmoothAngle(const QAngle& current, QAngle& target, float smoothFactor);

    uintptr_t GetBestTarget(float maxFov);
    bool IsValidTarget(uintptr_t entity);
    bool IsVisible(uintptr_t entity);
    Vector GetBonePosition(uintptr_t entity, int boneIndex);

    bool IsKeyPressed(int key);
    void ApplyRecoilControl(uintptr_t localPlayer, QAngle& angles);
}
