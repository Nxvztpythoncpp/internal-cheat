#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <wtypes.h>

namespace Unhook {
    void StartSafeCleanupThread();
    void RequestUnload();
    bool IsCleaning();
    void RegisterWorkerThread(std::thread t);
    bool ShouldWorkersStop();
    void SetModuleHandle(HMODULE mod);
    HMODULE GetModuleHandleInternal();
    void SetWindowInfo(HWND hWnd, WNDPROC oWndProc);
    void ResetGlobalState();
}
