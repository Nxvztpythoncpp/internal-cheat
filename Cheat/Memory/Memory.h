#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>

namespace Memory
{
    extern uintptr_t clientDll;
    extern uintptr_t engineDll;

    bool Initialize();

    template<typename T>
    T Read(uintptr_t address)
    {
        if (!address)
            return T();
        return *reinterpret_cast<T*>(address);
    }

    template<typename T>
    void Write(uintptr_t address, T value)
    {
        if (!address)
            return;
        *reinterpret_cast<T*>(address) = value;
    }
}
