// Memory.h
#pragma once
#include <Windows.h>
#include <TlHelp32.h>

namespace Memory {
    extern DWORD clientDll;
    extern DWORD engineDll;

    bool Initialize();

    template<typename T>
    T Read(uintptr_t address) {
        if (address == 0) return T();
        return *reinterpret_cast<T*>(address);
    }

    template<typename T>
    void Write(uintptr_t address, T value) {
        if (address == 0) return;
        *reinterpret_cast<T*>(address) = value;
    }
}