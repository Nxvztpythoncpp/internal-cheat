#include "ThirdPerson.h"

// TO DO LATER: Fix third person implementation

/*
void ThirdPerson::HandleToggle() {
    static bool pressedLast = false;
    bool pressedNow = GetAsyncKeyState(config_vars.tpKey);

    if (pressedNow && !pressedLast) {
        config_vars.tpEnabled = !config_vars.tpEnabled;
    }

    pressedLast = pressedNow;
}

void ThirdPerson::OverrideView(void* ecx, void* edx, void* pView) {
    Hooks::oOverrideView(ecx, edx, pView);

    if (!config_vars.tpEnabled)
        return;

    CViewSetup* view = (CViewSetup*)pView;
    CBaseEntity* local = (CBaseEntity*)Interfaces::EntityList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
    if (!local || !local->IsAlive())
        return;

    Vector viewAngles;
    Interfaces::Engine->GetViewAngles(viewAngles);
    Vector forward;
    Math::AngleVectors(viewAngles, &forward, nullptr, nullptr);

    view->origin = local->GetEyePosition() - (forward * 120.0f);
}
*/