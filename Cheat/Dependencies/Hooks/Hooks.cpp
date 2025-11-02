#define _CRT_SECURE_NO_WARNINGS
#include "Hooks.h"
#include "../../Cheats/Visuals/Visuals.h"
#include "../../Dependencies/MinHook/MinHook.h"
#include "../../Gui/imgui/imgui.h"
#include "../../Gui/imgui/backends/imgui_impl_dx9.h"
#include "../../Gui/imgui/backends/imgui_impl_win32.h"
#include "../../Cheats/Rage/Aimbot.h"
#include "../../Cheats/Misc/Movement.h"
#include "../../Cheats/Misc/UninjectHook.h"
#include "../../Gui/Menu/Menu.h"
#include <Windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <iostream>
#include "../../Cheats/SkinChanger/SkinChanger.h"
#include "../../Cheats/Misc/radar.h"
#include "../CacheSystem/CacheSystem.h"
#include "../../Dependencies/Csgo/SDK/HookSDK.h"
#include "../../Dependencies/Csgo/SDK/IVModelRender.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ===== Globals =====
LPDIRECT3DDEVICE9 g_pDevice = nullptr;
HWND g_hWindow = nullptr;
WNDPROC oWndProc = nullptr;
bool g_bInitialized = false;
bool d3d_init = false;
SetCursorPosHook oSetCursorPos = nullptr;
bool g_consoleAllocated = true;
EndScene oEndScene = nullptr;

namespace Hooks {
    bool menu_open = false;
    bool input_shouldListen = false;
    bool input_blocked = false;

    void BlockGameInput(bool block) {
        input_blocked = block;
        ImGuiIO& io = ImGui::GetIO();
        if (block) {
            io.WantCaptureMouse = true;
            io.WantCaptureKeyboard = true;
            io.MouseDrawCursor = true;
        }
        else {
            io.WantCaptureMouse = false;
            io.WantCaptureKeyboard = false;
            io.MouseDrawCursor = false;
        }
    }

    bool IsInputBlocked() {
        return input_blocked;
    }

    void SetWindowInfo(HWND hWnd, WNDPROC oWndProc) {
        Unhook::SetWindowInfo(hWnd, oWndProc);
    }
}

BOOL WINAPI hkSetCursorPos(int X, int Y) {
    if (Hooks::menu_open || Hooks::input_blocked) {
        return TRUE;
    }
    return oSetCursorPos(X, Y);
}

void InitImGui(LPDIRECT3DDEVICE9 pDevice) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = false;
    ImGui_ImplWin32_Init(g_hWindow);
    ImGui_ImplDX9_Init(pDevice);
}

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);
    if (GetCurrentProcessId() != wndProcId)
        return TRUE;
    g_hWindow = handle;
    return FALSE;
}

HWND GetProcessWindow() {
    g_hWindow = NULL;
    EnumWindows(EnumWindowsCallback, NULL);
    return g_hWindow;
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice) {
    if (Unhook::IsCleaning()) {
        return oEndScene(pDevice);
    }
    if (!g_bInitialized) {
        g_pDevice = pDevice;
        g_hWindow = GetProcessWindow();
        if (g_hWindow) {
            InitImGui(pDevice);
            oWndProc = (WNDPROC)SetWindowLongPtr(g_hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);
            Hooks::SetWindowInfo(g_hWindow, oWndProc);
            g_bInitialized = true;
            SetupSDKHooks();
        }
        else {
            return oEndScene(pDevice);
        }
    }

    if (!d3d_init) {
        auto& style = ImGui::GetStyle();
        style.WindowMinSize = ImVec2(10, 10);
        d3d_init = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UpdateFPS();
    Visuals::DrawHealthESP();
    Visuals::DrawBoxESP();
    Visuals::DrawSkeletonESP();
    StartCacheThread();
    StartFunctionsThread();
    Hooks::BlockGameInput(Hooks::menu_open);
    Menu::Draw();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    if (g_consoleAllocated) {
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
        g_consoleAllocated = false;
        std::cerr << "Console allocated successfully" << std::endl;
    }

    return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool insert_down_last = false;

    if (g_bInitialized) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
            return TRUE;
        }

        bool insert_down_now = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        if (insert_down_now && !insert_down_last) {
            Hooks::menu_open = !Hooks::menu_open;
            Hooks::BlockGameInput(Hooks::menu_open);
        }
        insert_down_last = insert_down_now;

        if (Hooks::menu_open || Hooks::input_blocked) {
            switch (uMsg) {
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
            case WM_MOUSEHWHEEL:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_XBUTTONDBLCLK:
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:
            case WM_INPUT:
                return TRUE;
            }
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool HookEndScene() {
    LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D)
        return false;

    D3DPRESENT_PARAMETERS pp = { 0 };
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = GetProcessWindow();
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;

    LPDIRECT3DDEVICE9 pDummyDevice = nullptr;
    HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &pDummyDevice);
    if (FAILED(hr) || !pDummyDevice) {
        pD3D->Release();
        return false;
    }

    void** vTable = *(void***)pDummyDevice;
    void* endSceneAddr = vTable[42];

    pDummyDevice->Release();
    pD3D->Release();

    if (MH_CreateHook(endSceneAddr, &hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene)) != MH_OK)
        return false;

    if (MH_EnableHook(endSceneAddr) != MH_OK)
        return false;

    return true;
}

void Initialize(HMODULE hModule) {
    freopen("debug.log", "w", stderr);

    if (MH_Initialize() != MH_OK) {
        std::cerr << "MinHook initialization failed" << std::endl;
        return;
    }

    HMODULE hUser32 = LoadLibraryA("user32.dll");
    if (hUser32) {
        void* pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
        if (pSetCursorPos && MH_CreateHook(pSetCursorPos, &hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos)) == MH_OK) {
            MH_EnableHook(pSetCursorPos);
        }
    }

    if (!HookEndScene()) {
        std::cerr << "Failed to hook EndScene" << std::endl;
        return;
    }

    std::cerr << "Hooks initialized successfully" << std::endl;
}