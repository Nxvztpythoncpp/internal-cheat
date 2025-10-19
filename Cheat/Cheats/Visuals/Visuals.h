#pragma once
#include <cstdint>

namespace Visuals {
    extern bool glowEnabled;
    extern bool glowThroughWalls;
    extern float glowColor[4];
    extern bool drawHealthBar;
    // --- New Box ESP config ---
    extern bool drawBoxes;      
    extern bool teamCheck;      
    extern float boxColor[4];   
    // --------------------------
    void Glow();
    void DrawHealthESP();
    void DrawBoxESP();          
}
