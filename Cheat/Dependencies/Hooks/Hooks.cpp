#define _CRT_SECURE_NO_WARNINGS

#include "Hooks.h"
#include "../../Cheats/Visuals/Visuals.h"
#include "../MinHook/MinHook.h"
#include "../../Gui/imgui/imgui.h"
#include "../../Gui/imgui/backends/imgui_impl_dx9.h"
#include "../../Gui/imgui/backends/imgui_impl_win32.h"
#include "../kiero/kiero.h"
#include "../../Gui/Menu/Menu.h"
#include <Windows.h>
#include <stdio.h>

#ifdef _WIN64
#define GWL_WNDPROC GWLP_WNDPROC
#endif


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static LPDIRECT3DDEVICE9 g_pDevice = nullptr;
static HWND g_hWindow = nullptr;
static WNDPROC oWndProc = nullptr;
static bool g_bInitialized = false;
static bool d3d_init = false;
SetCursorPosHook oSetCursorPos = nullptr;

bool nigga = true;

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
      
            // This prevents CS:GO from reading mouse movements directly !!!!!!!!!!!!!!!!!!!

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
}

EndScene oEndScene = nullptr;

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
    if (!g_bInitialized) {
        g_pDevice = pDevice;
        g_hWindow = GetProcessWindow();
        if (g_hWindow) {
            InitImGui(pDevice);
            oWndProc = (WNDPROC)SetWindowLongPtr(g_hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);
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

    Visuals::Glow();
    Visuals::DrawHealthESP(); //delete on finals releases

    Hooks::BlockGameInput(Hooks::menu_open);
    Menu::Draw();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());


    //delete on finals releases
    if(nigga=true)
    { 
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
        //printf("Debugging Window:\n");

        nigga = false;
    }

 

    return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool insert_down_last = false;

    if (g_bInitialized) {
		// Always let ImGui handle input first remember henrique
        if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
            return TRUE;
        }

        bool insert_down_now = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        if (insert_down_now && !insert_down_last) {
            Hooks::menu_open = !Hooks::menu_open;
            Hooks::BlockGameInput(Hooks::menu_open);
        }
        insert_down_last = insert_down_now;

        // Block ALL input to the game when menu is open
        if (Hooks::menu_open || Hooks::input_blocked) {
            switch (uMsg) {
                // Mouse events
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

                // Keyboard events (prevent shooting/movement)
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_CHAR:
            case WM_DEADCHAR:
            case WM_SYSCHAR:
            case WM_SYSDEADCHAR:

                // RAW INPUT - MUSTHAVE BITCH FOR CS:GO
            case WM_INPUT:
                if (Hooks::menu_open || Hooks::input_blocked) {
                    return TRUE;
                }
                break; 
            }
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void Initialize(HMODULE hModule) {
    if (MH_Initialize() == MH_OK) {
        printf("ThreadProc: Initialize wdkhlçajukluhjndwqauhjikladswUIOLPÇDSWA\n");

        HMODULE hUser32 = LoadLibraryA("user32.dll");
        if (hUser32) {
            void* pSetCursorPos = GetProcAddress(hUser32, "SetCursorPos");
            if (pSetCursorPos && MH_CreateHook(pSetCursorPos, &hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos)) == MH_OK) {
                MH_EnableHook(pSetCursorPos);
            }
        }
    }

    if (kiero::init(kiero::RenderType::D3D9) == kiero::Status::Success) {
        kiero::bind(42, (void**)&oEndScene, hkEndScene);
    }
}