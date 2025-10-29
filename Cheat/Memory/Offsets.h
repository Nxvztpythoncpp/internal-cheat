#pragma once
#include <cstdint>

// Unified Offsets (custom + hazedumper)
// Updated: 2025-10-18 21:11:12 UTC

namespace offsets
{
    // --- Client DLL Offsets (from hazedumper::signatures)
    constexpr std::uintptr_t dwLocalPlayer = 0xDEF97C;
    constexpr std::uintptr_t dwEntityList = 0x4E051DC;
    constexpr std::uintptr_t dwClientState = 0x59F19C;
    constexpr std::uintptr_t dwClientState_ViewAngles = 0x4D90;
    constexpr std::uintptr_t dwClientState_GetLocalPlayer = 0x180;
    constexpr std::uintptr_t dwClientState_State = 0x108;
    constexpr std::uintptr_t dwClientState_MaxPlayer = 0x388;
    constexpr std::uintptr_t dwClientState_PlayerInfo = 0x52C0;
    constexpr std::uintptr_t dwClientState_Map = 0x28C;
    constexpr std::uintptr_t dwClientState_MapDirectory = 0x188;
    constexpr std::uintptr_t dwGlowObjectManager = 0x535FCB8;
    constexpr std::uintptr_t dwForceAttack = 0x3233024;
    constexpr std::uintptr_t dwForceJump = 0x52C0F50;
    constexpr std::uintptr_t dwInput = 0x52627B0;
    constexpr std::uintptr_t dwViewMatrix = 0x4DF6024;
    constexpr std::uintptr_t dwGlobalVars = 0x59EE60;

    // --- Entity Offsets (from hazedumper::netvars)
    constexpr std::uintptr_t m_iHealth = 0x100;
    constexpr std::uintptr_t m_iTeamNum = 0xF4;
    constexpr std::uintptr_t m_iCrosshairId = 0x11838;
    constexpr std::uintptr_t m_vecOrigin = 0x138;
    constexpr std::uintptr_t m_vecViewOffset = 0x108;
    constexpr std::uintptr_t m_dwBoneMatrix = 0x26A8;
    constexpr std::uintptr_t m_aimPunchAngle = 0x303C;
    constexpr std::uintptr_t m_bSpottedByMask = 0x980;
    constexpr std::uintptr_t m_bDormant = 0xED;
    constexpr std::uintptr_t m_lifeState = 0x25F;
    constexpr std::uintptr_t m_fFlags = 0x104;
    constexpr std::uintptr_t m_hActiveWeapon = 0x2F08;
    constexpr std::uintptr_t m_iShotsFired = 0x103E0;
    constexpr std::uintptr_t m_angEyeAnglesX = 0x117D0;
    constexpr std::uintptr_t m_angEyeAnglesY = 0x117D4;
    constexpr std::uintptr_t m_iGlowIndex = 0x10488;
    constexpr std::uintptr_t m_flSimulationTime = 0x268;
    constexpr std::uintptr_t m_flFlashDuration = 0x10470;
    constexpr std::uintptr_t m_iDefaultFOV = 0x333C;
    constexpr std::uintptr_t m_hViewModel = 0x3308;
    constexpr std::uintptr_t m_szLastPlaceName = 0x35C4;

    // --- Weapon Offsets (from hazedumper::netvars)
    constexpr std::uintptr_t m_iClip1 = 0x3274;
    constexpr std::uintptr_t m_iItemDefinitionIndex = 0x2FBA;
    constexpr std::uintptr_t m_iItemIDHigh = 0x2FD0;
    constexpr std::uintptr_t m_nFallbackPaintKit = 0x31D8;
    constexpr std::uintptr_t m_nFallbackSeed = 0x31DC;
    constexpr std::uintptr_t m_flFallbackWear = 0x31E0;
    constexpr std::uintptr_t m_nFallbackStatTrak = 0x31E4;
    constexpr std::uintptr_t m_szCustomName = 0x304C;
    constexpr std::uintptr_t m_hMyWeapons = 0x2E08;
    constexpr std::uintptr_t m_iEntityQuality = 0x2FBC;
    constexpr std::uintptr_t m_nModelIndex = 0x258;
    constexpr std::uintptr_t m_nViewModelIndex = 0x29D0;
    constexpr std::uintptr_t m_iAccountID = 0x2FD8;
    constexpr std::uintptr_t m_OriginalOwnerXuidLow = 0x31D0;
    constexpr std::uintptr_t m_OriginalOwnerXuidHigh = 0x31D4;

    // --- Additional known (custom)
    constexpr std::uintptr_t update_client_side_animation = 0x3A4C50;
    constexpr std::uintptr_t setup_bones = 0x3A4C80;
    constexpr std::uintptr_t standard_blending_rules = 0x3A4CB0;
    constexpr std::uintptr_t calc_absolute_position = 0x3A4CE0;
    constexpr std::uintptr_t invalidate_bone_cache = 0x3A4D10;
    constexpr std::uintptr_t write_user_cmd = 0x3A4D40;
    constexpr std::uintptr_t glow_object = 0x535BAD0;
    constexpr std::uintptr_t prediction_random_seed = 0x3A4D70;
    constexpr std::uintptr_t prediction_player = 0x3A4DA0;
    constexpr std::uintptr_t move_helper = 0x3A4DD0;
    constexpr std::uintptr_t global_vars = 0x3A4E00;
    constexpr std::uintptr_t client_side_animation_list = 0x3A4E30;
    constexpr std::uintptr_t create_animstate = 0x3A4E60;
    constexpr std::uintptr_t reset_animstate = 0x3A4E90;
    constexpr std::uintptr_t update_animstate = 0x3A4EC0;
    constexpr std::uintptr_t set_abs_angles = 0x1EA950;
    constexpr std::uintptr_t set_abs_origin = 0x1EA790;


    constexpr std::uintptr_t clientstate_delta_ticks = 0x174;
}

