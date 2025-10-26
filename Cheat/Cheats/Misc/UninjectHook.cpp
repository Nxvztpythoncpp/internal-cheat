#define _CRT_SECURE_NO_WARNINGS

#include "UninjectHook.h"
#include <mutex>
#include <chrono>
#include <iostream>
#include "../../Gui/Imgui/backends/imgui_impl_dx9.h"
#include "../../Gui/Imgui/backends/imgui_impl_win32.h"
#include "../../Dependencies/Hooks/Hooks.h"
#include "../../Dependencies/MinHook/MinHook.h"

namespace {
    std::atomic<bool> g_isCleaning(false);
    std::atomic<HMODULE> g_moduleHandle(nullptr);
    std::mutex g_threadsMutex;
    std::vector<std::thread> g_workerThreads;
    HWND g_window = nullptr;
    WNDPROC g_oldWndProc = nullptr;
}

namespace Unhook {

    void CleanupImGui() {
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX9_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void SetModuleHandle(HMODULE mod) {
        g_moduleHandle = mod;
    }

    HMODULE GetModuleHandleInternal() {
        return (HMODULE)g_moduleHandle.load();
    }

    void RegisterWorkerThread(std::thread t) {
        std::lock_guard<std::mutex> lock(g_threadsMutex);
        g_workerThreads.emplace_back(std::move(t));
    }

    bool IsCleaning() {
        return g_isCleaning.load();
    }

    bool ShouldWorkersStop() {
        return g_isCleaning.load();
    }

    void RequestUnload() {
        g_isCleaning.store(true);
    }

    void SetWindowInfo(HWND hWnd, WNDPROC oWndProc) {
        g_window = hWnd;
        g_oldWndProc = oWndProc;
    }

    void ResetGlobalState() {
        g_isCleaning.store(false);
        g_moduleHandle.store(nullptr);
        g_window = nullptr;
        g_oldWndProc = nullptr;
        g_threadsMutex.lock();
        g_workerThreads.clear();
        g_threadsMutex.unlock();
        Hooks::menu_open = false;
        Hooks::input_blocked = false;
        g_pDevice = nullptr;
        g_hWindow = nullptr;
        oWndProc = nullptr;
        g_bInitialized = false;
        d3d_init = false;
        g_consoleAllocated = true;
    }

    void StartSafeCleanupThread() {
        Hooks::menu_open = false;
        Hooks::input_blocked = false;
        std::thread cleanupThread([]() {
            freopen("debug.log", "a", stderr);

            for (int i = 0; i < 100 && !g_isCleaning.load(); ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            {
                std::lock_guard<std::mutex> lock(g_threadsMutex);
                for (auto& t : g_workerThreads) {
                    if (t.joinable()) {
                        try {
                            t.join();
                        }
                        catch (...) {
                            std::cerr << "Exception while joining thread" << std::endl;
                        }
                    }
                }
                g_workerThreads.clear();
            }

            if (g_window && g_oldWndProc) {
                CleanupImGui();
                if (SetWindowLongPtr(g_window, GWL_WNDPROC, (LONG_PTR)g_oldWndProc) == 0) {
                    std::cerr << "Failed to restore WNDPROC: " << GetLastError() << std::endl;
                }
                else {
                    std::cerr << "WNDPROC restored successfully" << std::endl;
                }
            }

            HMODULE hUser32 = GetModuleHandleA("user32.dll");
            if (hUser32) {
                void* pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
                if (pSetCursorPos) {
                    MH_DisableHook(pSetCursorPos);
                }
            }
            LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
            if (pD3D) {
                D3DPRESENT_PARAMETERS pp = { 0 };
                pp.Windowed = TRUE;
                pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
                pp.hDeviceWindow = GetProcessWindow();
                pp.BackBufferFormat = D3DFMT_A8R8G8B8;
                LPDIRECT3DDEVICE9 pDummyDevice = nullptr;
                if (SUCCEEDED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pp.hDeviceWindow,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pDummyDevice)) && pDummyDevice) {
                    void** vTable = *(void***)pDummyDevice;
                    void* endSceneAddr = vTable[42];
                    MH_DisableHook(endSceneAddr);
                    pDummyDevice->Release();
                }
                pD3D->Release();
            }

            MH_STATUS status = MH_Uninitialize();
            if (status != MH_OK) {
                std::cerr << "Failed to uninitialize MinHook: " << status << std::endl;
            }
            else {
                std::cerr << "MinHook uninitialized successfully" << std::endl;
            }

            HMODULE minHookModule = GetModuleHandleA("minhook.x86.dll");
            if (minHookModule) {
                if (FreeLibrary(minHookModule)) {
                    std::cerr << "Successfully unloaded minhook.x86.dll" << std::endl;
                }
                else {
                    std::cerr << "Failed to unload minhook.x86.dll: " << GetLastError() << std::endl;
                }
            }

            if (GetConsoleWindow()) {
                if (fclose(stdin) != 0) {
                    std::cerr << "Failed to close stdin: " << GetLastError() << std::endl;
                }
                if (fclose(stdout) != 0) {
                    std::cerr << "Failed to close stdout: " << GetLastError() << std::endl;
                }
                if (fclose(stderr) != 0) {
                    std::cerr << "Failed to close stderr: " << GetLastError() << std::endl;
                }
                if (freopen("NUL:", "r", stdin) == nullptr ||
                    freopen("NUL:", "w", stdout) == nullptr ||
                    freopen("NUL:", "w", stderr) == nullptr) {
                    std::cerr << "Failed to reassign console handles" << std::endl;
                }
                if (!FreeConsole()) {
                    std::cerr << "Failed to free console: " << GetLastError() << std::endl;
                }
                else {
                    std::cerr << "Console freed successfully" << std::endl;
                }
            }

            HMODULE mod = (HMODULE)g_moduleHandle.load();
            if (mod) {
                BOOL res = FreeLibrary(mod);
                if (!res) {
                    std::cerr << "Failed to free library: " << GetLastError() << std::endl;
                }
                else {
                    std::cerr << "Successfully unloaded DLL" << std::endl;
                }
            }

            ResetGlobalState();
            std::cerr << "Cleanup completed, ready for reinjection" << std::endl;
            });

        cleanupThread.detach();
    }
}