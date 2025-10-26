#pragma once
#include <cstdint>

namespace Visuals {
    extern bool espEnabled;
    extern bool glowEnabled;
    extern bool glowThroughWalls;
    extern float glowColorEnemy[4];
    extern float glowColorTeam[4];
    extern bool drawHealthBar;
    // --- New Box ESP config ---
    extern bool drawBoxes; 
    extern bool boxOutline;
    extern bool teamCheck;      
    extern float boxColor[4];   
    // --------------------------
    void Glow();
    void DrawHealthESP();
    void DrawBoxESP();          
}
