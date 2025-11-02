// CacheSystem.h
#ifndef CACHE_SYSTEM_H
#define CACHE_SYSTEM_H
#include <vector>
#include <array>
#include <cstdint>
#include <algorithm> // for std::fill
#include <Windows.h>

typedef struct { float x, y, z; } Vec3;  // Align with Visuals.cpp

struct PlayerCache {
    uintptr_t entity;
    int entityID;
    int health;
    int team;
    Vec3 origin;
    bool isSpectator;
    bool isDormant;
    // Resolver additions for desync/AA
    float lby;  // Lower Body Yaw
    float eyeYaw;  // Eye angles yaw (server)
    float serverVelocityLength2D;  // Server-side speed
    bool isDesynced;  // Detection flag
    int resolveSide;  // -1 left, 0 center, 1 right (computed on-fly)

    PlayerCache()
        : entity(0), entityID(0), health(0), team(0),
        origin{ 0.0f, 0.0f, 0.0f },
        isSpectator(false), isDormant(false),
        lby(0.0f), eyeYaw(0.0f), serverVelocityLength2D(0.0f),
        isDesynced(false), resolveSide(0)
    {
    }
    bool IsTeammate(int localTeam) const { return team == localTeam && health > 0 && !isSpectator && !isDormant; }
    bool IsEnemy(int localTeam) const { return team != localTeam && health > 0 && !isSpectator && !isDormant; }
};

struct EntityCache {
    std::vector<PlayerCache> teammates;
    std::vector<PlayerCache> enemies;
    std::vector<PlayerCache> spectators;
    std::array<int, 64> teammateIDs{};
    std::array<int, 64> enemyIDs{};
    std::array<int, 64> spectatorIDs{};
    int numTeammates = 0;
    int numEnemies = 0;
    int numSpectators = 0;
    int localTeam = 0;
    uintptr_t localPlayer = 0;
    void Clear() {
        teammates.clear();
        enemies.clear();
        spectators.clear();
        numTeammates = numEnemies = numSpectators = 0;
        std::fill(teammateIDs.begin(), teammateIDs.end(), 0);
        std::fill(enemyIDs.begin(), enemyIDs.end(), 0);
        std::fill(spectatorIDs.begin(), spectatorIDs.end(), 0);
    }
};
extern EntityCache g_Cache;
// Function declarations
void UpdateCache(uintptr_t moduleBase);
DWORD WINAPI CacheThread(LPVOID param);
// FPS globals
extern LARGE_INTEGER g_PerfFreq;
extern LARGE_INTEGER g_LastFrameTime;
extern float g_CurrentFPS;
void UpdateFPS();
void StartCacheThread();
void StartFunctionsThread();
void StopCacheThread();
void StopFunctionsThread();
#endif // CACHE_SYSTEM_H