// CacheSystem.cpp
#define NOMINMAX
#include "CacheSystem.h"
#include "..\..\Memory\Memory.h"
#include "..\..\Memory\Offsets.h"
#include <Windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cmath> // for round()
#include "../../Cheats/Visuals/Visuals.h"
#include "../../Cheats/Misc/Movement.h"
#include "../../Cheats/Rage/Aimbot.h"
#include "../../Cheats/SkinChanger/SkinChanger.h"
#include "../../Cheats/Misc/radar.h"

LARGE_INTEGER g_PerfFreq = {};
LARGE_INTEGER g_LastFrameTime = {};
float g_CurrentFPS = 0.0f;
EntityCache g_Cache{};
std::atomic<bool> g_Running{ false };
std::atomic<bool> f_Running{ false };
std::thread g_Thread;
std::thread f_Thread;

static int ClampInt(int value, int minVal, int maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void UpdateCache(uintptr_t moduleBase) {
    try {
        g_Cache.Clear();
        if (!moduleBase) return;
        uintptr_t localPlayer = Memory::Read<uintptr_t>(moduleBase + offsets::dwLocalPlayer);
        if (!localPlayer) return;
        g_Cache.localPlayer = localPlayer;
        g_Cache.localTeam = Memory::Read<int>(localPlayer + offsets::m_iTeamNum);
        uintptr_t entityList = moduleBase + offsets::dwEntityList;
        int scanned = 0;
        for (int i = 1; i <= 64; ++i) {  // Fixed: Start at 1, use i * 0x10
            uintptr_t entity = Memory::Read<uintptr_t>(entityList + i * 0x10);
            scanned++;
            if (!entity || entity == localPlayer) continue;
            PlayerCache p;
            p.entity = entity;
            p.entityID = i;
            p.health = Memory::Read<int>(entity + offsets::m_iHealth);
            p.team = Memory::Read<int>(entity + offsets::m_iTeamNum);
            p.isDormant = Memory::Read<bool>(entity + offsets::m_bDormant);
            p.isSpectator = Memory::Read<int>(entity + offsets::m_iObserverMode) > 0;
            p.origin = Memory::Read<Vec3>(entity + offsets::m_vecOrigin);  // Use Vec3
            // Resolver data (for desync/AA)
            p.lby = Memory::Read<float>(entity + offsets::m_flLowerBodyYawTarget);
            float eyeAngles[3];
            eyeAngles[0] = Memory::Read<float>(entity + offsets::m_angEyeAnglesX);  // Pitch
            eyeAngles[1] = Memory::Read<float>(entity + offsets::m_angEyeAnglesY);  // Yaw
            eyeAngles[2] = Memory::Read<float>(entity + offsets::m_angEyeAnglesZ);  // Roll
            p.eyeYaw = eyeAngles[1];
            float vel[3];
            vel[0] = Memory::Read<float>(entity + offsets::m_vecVelocity + 0x0);
            vel[1] = Memory::Read<float>(entity + offsets::m_vecVelocity + 0x4);
            vel[2] = Memory::Read<float>(entity + offsets::m_vecVelocity + 0x8);
            p.serverVelocityLength2D = sqrtf(vel[0] * vel[0] + vel[1] * vel[1]);  // 2D speed
            // Basic desync detect
            p.isDesynced = (p.serverVelocityLength2D < 1.0f) && (fabsf(p.lby - p.eyeYaw) > 35.0f);
            // printf(" slot=%d addr=0x%08X team=%d health=%d dormant=%d desync=%d\n",
            //     i, (unsigned int)entity, p.team, p.health, p.isDormant, p.isDesynced);
            if (p.health <= 0 || p.health > 100) continue;
            if (p.IsTeammate(g_Cache.localTeam)) {
                g_Cache.teammates.push_back(p);
                g_Cache.teammateIDs[g_Cache.numTeammates++] = i;
            }
            else if (p.IsEnemy(g_Cache.localTeam)) {
                g_Cache.enemies.push_back(p);
                g_Cache.enemyIDs[g_Cache.numEnemies++] = i;
            }
            else if (p.isSpectator) {
                g_Cache.spectators.push_back(p);
                g_Cache.spectatorIDs[g_Cache.numSpectators++] = i;
            }
        }
        // printf("[CACHE] Updated: enemies=%d teammates=%d spectators=%d scanned=%d\n",
        //     g_Cache.numEnemies, g_Cache.numTeammates, g_Cache.numSpectators, scanned);
    }
    catch (...) {
        printf("[CACHE] Update exception\n");
    }
}

DWORD WINAPI CacheThread(LPVOID) {
    printf("[CACHE] CacheThread started\n");
    while (g_Running) {
        if (Memory::clientDll) {
            UpdateCache(Memory::clientDll);
        }
        int sleepMs = 5;
        if (g_CurrentFPS > 0.0f) {
            double intended = 1000.0 / g_CurrentFPS;
            sleepMs = ClampInt((int)std::round(intended), 1, 50);
        }
        Sleep(sleepMs);
    }
    printf("[CACHE] CacheThread exiting\n");
    return 0;
}

DWORD WINAPI FunctionsThread(LPVOID) {
    printf("[FUNCTIONS] FunctionsThread started\n");
    while (f_Running) {
        if (Memory::clientDll == 0) {
            if (!Memory::Initialize()) {
                Sleep(100);
                continue;
            }
        }

        // Run cheats only if initialized
        if (Memory::clientDll != 0) {
            Visuals::Glow();
            Movement::BunnyHop();
            Aimbot::Run();
            //SkinChanger::Run();
            Radar::Run();
        }

        // Existing sleep logic...
        int sleepMs = 5;
        if (g_CurrentFPS > 0.0f) {
            double intended = 1000.0 / g_CurrentFPS;
            sleepMs = ClampInt((int)std::round(intended), 1, 50);
        }
        Sleep(sleepMs);
    }
    printf("[FUNCTIONS] FunctionsThread exiting\n");
    return 0;
}

void UpdateFPS() {
    if (g_PerfFreq.QuadPart == 0)
        QueryPerformanceFrequency(&g_PerfFreq);
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (g_LastFrameTime.QuadPart != 0) {
        double delta = double(now.QuadPart - g_LastFrameTime.QuadPart) / g_PerfFreq.QuadPart;
        if (delta > 0.0001 && delta < 0.1)
            g_CurrentFPS = (float)(1.0 / delta);
    }
    g_LastFrameTime = now;
}

void StartCacheThread() {
    if (g_Running) return;
    g_Running = true;
    g_Thread = std::thread(CacheThread, nullptr);  // Use std::thread for better control
    printf("[CACHE] Cache thread created\n");
}

void StartFunctionsThread() {
    if (f_Running) return;
    f_Running = true;
    f_Thread = std::thread(FunctionsThread, nullptr);
    printf("[FUNCTIONS] Functions thread created\n");
}

void StopCacheThread() {
    g_Running = false;
    if (g_Thread.joinable()) g_Thread.join();
}

void StopFunctionsThread() {
    f_Running = false;
    if (f_Thread.joinable()) f_Thread.join();
}