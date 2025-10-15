#pragma once

#include <Windows.h>

typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

inline void* GetInterface(const char* moduleName, const char* interfaceName) {
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (!hModule) {
        // Adiciona depuração se o módulo não for encontrado
        // Exemplo: OutputDebugStringA("Módulo não encontrado: " + std::string(moduleName) + "\n");
        return nullptr;
    }

    CreateInterfaceFn createInterface = (CreateInterfaceFn)GetProcAddress(hModule, "CreateInterface");
    if (!createInterface) {
        // Adiciona depuração se CreateInterface não for encontrado
        // Exemplo: OutputDebugStringA("CreateInterface não encontrado em " + std::string(moduleName) + "\n");
        return nullptr;
    }

    int returnCode;
    void* interfacePtr = createInterface(interfaceName, &returnCode);
    return interfacePtr;
}