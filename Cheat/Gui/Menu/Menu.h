#pragma once

#include <Windows.h>

namespace Menu {
    void Draw();
    void Toggle();
    bool IsVisible();
    void CaptureInput(bool capture);
}