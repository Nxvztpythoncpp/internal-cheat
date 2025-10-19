#pragma once
#include <cstdint>

namespace Visuals {
    extern bool glowEnabled;
    extern bool glowThroughWalls;
    extern float glowColor[4];
    extern bool drawHealthBar;

    void Glow();
    void DrawHealthESP();
}
