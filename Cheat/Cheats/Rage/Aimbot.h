#pragma once
#include "../../Memory/Offsets.h"
#include <Windows.h>
#include <cmath>
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
    int GetBestTarget(float maxFov);
    bool IsValidTarget(int entity);
    bool IsVisible(int entity);
    Vector GetBonePosition(int entity, int boneIndex);
    bool IsKeyPressed(int key);
    void ApplyRecoilControl(int localPlayer, QAngle& angles);
}