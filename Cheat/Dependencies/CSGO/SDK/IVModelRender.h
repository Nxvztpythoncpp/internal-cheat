#pragma once
#include "../../Hooks/Hooks.h"

//pretty stable but needs more testing after fixing other parts of the cheat

struct DrawModelState_t {
    void* studioHdr;
    void* studioHWData;
    void* decals;
    int drawFlags;
    int lod;
};

struct ModelRenderInfo_t {
    Vector origin;
    Vector angles;
    void* pRenderable;
    const void* pModel;
    const float* pLightingOffset;
    const float* pLightingOrigin;
    int flags;
    int entity_index;
    int skin;
    int body;
    int hitboxset;
    unsigned short instance;
    void* modelToWorld;
    void* lightingState;
    void* envCubemap;
};

class IVModelRender {
public:
    void DrawModelExecute(
        void* ctx,
        const DrawModelState_t& state,
        const ModelRenderInfo_t& info,
        float* matrix
    ) {
        using Fn = void(__thiscall*)(void*, void*, const DrawModelState_t&, const ModelRenderInfo_t&, float*);
        return (*reinterpret_cast<Fn**>(this))[21](this, ctx, state, info, matrix);
    }
};
