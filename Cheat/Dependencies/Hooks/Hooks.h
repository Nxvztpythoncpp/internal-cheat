#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d9.h>

struct QAngle {
    float x, y, z;
    QAngle() : x(0), y(0), z(0) {}
    QAngle(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    QAngle operator+(const QAngle& other) const {
        return QAngle(x + other.x, y + other.y, z + other.z);
    }

    QAngle operator-(const QAngle& other) const {
        return QAngle(x - other.x, y - other.y, z - other.z);
    }

    QAngle operator*(float scalar) const {
        return QAngle(x * scalar, y * scalar, z * scalar);
    }

    QAngle operator/(float scalar) const {
        return QAngle(x / scalar, y / scalar, z / scalar);
    }
};

struct Vector {
    float x, y, z;
    Vector() : x(0), y(0), z(0) {}
    Vector(float X, float Y, float Z) : x(X), y(Y), z(Z) {}

    Vector operator+(const Vector& other) const {
        return Vector(x + other.x, y + other.y, z + other.z);
    }

    Vector operator-(const Vector& other) const {
        return Vector(x - other.x, y - other.y, z - other.z);
    }

    Vector operator*(float scalar) const {
        return Vector(x * scalar, y * scalar, z * scalar);
    }

    Vector operator/(float scalar) const {
        return Vector(x / scalar, y / scalar, z / scalar);
    }
};

inline Vector QAngleToVector(const QAngle& angle) {
    return Vector(angle.x, angle.y, angle.z);
}

inline QAngle VectorToQAngle(const Vector& vec) {
    return QAngle(vec.x, vec.y, vec.z);
}

#define IN_ATTACK      (1 << 0)
#define IN_JUMP        (1 << 1)
#define IN_DUCK        (1 << 2)
#define IN_FORWARD     (1 << 3)
#define IN_BACK        (1 << 4)
#define IN_USE         (1 << 5)
#define IN_CANCEL      (1 << 6)
#define IN_LEFT        (1 << 7)
#define IN_RIGHT       (1 << 8)
#define IN_MOVELEFT    (1 << 9)
#define IN_MOVERIGHT   (1 << 10)
#define IN_ATTACK2     (1 << 11)

struct CUserCmd {
    int command_number;
    int tick_count;
    QAngle viewangles;
    Vector aimdirection;
    float forwardmove;
    float sidemove;
    float upmove;
    int buttons;
    unsigned char impulse;
    int weaponselect;
    int weaponsubtype;
    int random_seed;
    short mousedx;
    short mousedy;
    bool hasbeenpredicted;
};

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9 pDevice);
typedef BOOL(WINAPI* SetCursorPosHook)(int X, int Y);

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Initialize(HMODULE hModule);

extern SetCursorPosHook oSetCursorPos;

namespace Hooks {
    extern bool menu_open;
    extern bool input_shouldListen;

    //mouse fix (dantez)
    void BlockGameInput(bool block);
    bool IsInputBlocked();
}

void SetupSDKHooks();

template <typename T>
__forceinline T GetVFunc(void* base, size_t index) {
    return (T)((*(void***)base)[index]);
}