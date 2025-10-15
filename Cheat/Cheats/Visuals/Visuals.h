#pragma once
#include <cstdint>

namespace Visuals {
    // Configuration flags and color for glow
    extern bool glowEnabled;
    extern bool glowThroughWalls;
    extern float glowColor[4];  // RGBA color for glow (values 0.0f to 1.0f)

    // Function to apply glow ESP each frame
    void Glow();
}
