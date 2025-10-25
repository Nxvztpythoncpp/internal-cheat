#pragma once

#include <atomic>
#include <thread>

// Se tens um header do menu, ajusta a include path conforme necessário.
// #include "Menu.h"

namespace Unhook {

    void StartSafeCleanupThread();
    void RequestUnload();
    bool IsCleaning();
    void RegisterWorkerThread(std::thread t);
    bool ShouldWorkersStop();
    void UnbindHooks();
    void GuiUnhook();

}
