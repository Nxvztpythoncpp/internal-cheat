#include "../../Cheats/Config/config_vars.h"
#include "../../Dependencies/Hooks/Hooks.h"
//#include "../../Dependencies/CSGO/SDK/HookSDK.h
//#include "../../../Utils/Utils.h"
#include "Chams.h"
#include "../../Dependencies/CSGO/SDK/IVModelRender.h"


/*
void Chams::Init() {
    // Optional: Preload materials
}

void Chams::OnDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone) {
    if (!g_cfg.misc.chamsEnabled) return;

    auto mdl = Interfaces::ModelInfo->GetModel(info.model);
    const char* mdlName = Interfaces::ModelInfo->GetModelName(mdl);

    if (strstr(mdlName, "models/player")) {
        C_BaseEntity* ent = Interfaces::EntityList->GetClientEntity(info.entity_index);
        if (!ent || !ent->IsAlive() || ent->IsDormant()) return;

        if (g_cfg.misc.teamCheck && ent->GetTeam() == localPlayer->GetTeam())
            return;

        // Set colors
        Color visColor = g_cfg.misc.chamsVisibleColor;
        Color xrayColor = g_cfg.misc.chamsXrayColor;

        Interfaces::RenderView->SetColorModulation(xrayColor.Base());
        Interfaces::ModelRender->ForcedMaterialOverride(YourCustomMaterial, OVERRIDE_NORMAL);
        Hooks::originalDrawModelExecute(Interfaces::ModelRender, ctx, state, info, bone);

        // Visible
        Interfaces::RenderView->SetColorModulation(visColor.Base());
        Interfaces::ModelRender->ForcedMaterialOverride(YourVisibleMaterial, OVERRIDE_NORMAL);
        Hooks::originalDrawModelExecute(Interfaces::ModelRender, ctx, state, info, bone);

        Interfaces::ModelRender->ForcedMaterialOverride(nullptr); // reset
    }
}
*/