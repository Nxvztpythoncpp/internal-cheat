#include <chrono> 
#include <Windows.h>  
#include <cstring>   
#include <cstdio>    

/*
client.dll+3DE269 - 88 5C D1 28           - mov [ecx+edx*8+28],bl
client.dll+3DE2CF - F3 0F11 44 C8 08      - movss [eax+ecx*8+08],xmm0
client.dll+3DE3B7 - F3 0F11 44 D1 14      - movss [ecx+edx*8+14],xmm0
client.dll+3DE2DA - F3 0F11 44 C8 0C      - movss [eax+ecx*8+0C],xmm0   <--- this one is for team mates
*/

static BYTE origBytes1[4] = {};
static BYTE origBytes2[6] = {};
static BYTE origBytes3[6] = {};
static BYTE origBytes4[6] = {};
static bool isPatched = false;
static bool isTeamPatched = false;

void NopBytes(DWORD addr, size_t size) {
    if (!addr || size == 0) {
        printf("[PATCH] Invalid addr/size for NOP\n");
        return;
    }

    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)addr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        printf("[PATCH] VirtualProtect failed for NOP: %d\n", GetLastError());
        return;
    }

    BYTE nops[16] = { 0 };
    memset(nops, 0x90, size);
    for (size_t i = 0; i < size; ++i) {
        Memory::Write<BYTE>(addr + i, nops[i]);
    }

    DWORD dummy;
    VirtualProtect((LPVOID)addr, size, oldProtect, &dummy);
    printf("[PATCH] NOPed %zu bytes at 0x%08X\n", size, addr);
}

void NopGlowPatches() {
    if (isPatched || !Memory::clientDll) {
        printf("[PATCH] Already patched or invalid clientDll (0x%08X)\n", (DWORD)Memory::clientDll);
        return;
    }

    DWORD base = (DWORD)Memory::clientDll;
    DWORD addr1 = base + 0x3DE269;
    DWORD addr2 = base + 0x3DE2CF;
    DWORD addr3 = base + 0x3DE3B7;

    printf("[PATCH] Starting NOP: base=0x%08X\n", base);

    DWORD old1;
    if (VirtualProtect((LPVOID)addr1, 4, PAGE_EXECUTE_READWRITE, &old1)) {
        for (int i = 0; i < 4; ++i) origBytes1[i] = Memory::Read<BYTE>(addr1 + i);
        printf("[PATCH] Backup1: %02X %02X %02X %02X\n", origBytes1[0], origBytes1[1], origBytes1[2], origBytes1[3]);
        DWORD dummy1; VirtualProtect((LPVOID)addr1, 4, old1, &dummy1);
    }
    else {
        printf("[PATCH] Backup1 protect failed: %d\n", GetLastError());
        return;
    }

    DWORD old2;
    if (VirtualProtect((LPVOID)addr2, 6, PAGE_EXECUTE_READWRITE, &old2)) {
        for (int i = 0; i < 6; ++i) origBytes2[i] = Memory::Read<BYTE>(addr2 + i);
        printf("[PATCH] Backup2: %02X %02X %02X %02X %02X %02X\n", origBytes2[0], origBytes2[1], origBytes2[2], origBytes2[3], origBytes2[4], origBytes2[5]);
        DWORD dummy2; VirtualProtect((LPVOID)addr2, 6, old2, &dummy2);
    }
    else {
        printf("[PATCH] Backup2 protect failed: %d\n", GetLastError());
        return;
    }

    DWORD old3;
    if (VirtualProtect((LPVOID)addr3, 6, PAGE_EXECUTE_READWRITE, &old3)) {
        for (int i = 0; i < 6; ++i) origBytes3[i] = Memory::Read<BYTE>(addr3 + i);
        printf("[PATCH] Backup3: %02X %02X %02X %02X %02X %02X\n", origBytes3[0], origBytes3[1], origBytes3[2], origBytes3[3], origBytes3[4], origBytes3[5]);
        DWORD dummy3; VirtualProtect((LPVOID)addr3, 6, old3, &dummy3);
    }
    else {
        printf("[PATCH] Backup3 protect failed: %d\n", GetLastError());
        return;
    }

    NopBytes(addr1, 4);
    NopBytes(addr2, 6);
    NopBytes(addr3, 6);

    isPatched = true;
    printf("[PATCH] Glow patches NOPed successfully\n");
}

void NopTeamGlowPatches() {
    if (isTeamPatched || !Memory::clientDll) {
        printf("[PATCH] Already patched or invalid clientDll (0x%08X)\n", (DWORD)Memory::clientDll);
        return;
    }
    DWORD base = (DWORD)Memory::clientDll;
    DWORD addr4 = base + 0x3DE2DA;
    DWORD old4;
    if (VirtualProtect((LPVOID)addr4, 6, PAGE_EXECUTE_READWRITE, &old4)) {
        for (int i = 0; i < 6; ++i) origBytes4[i] = Memory::Read<BYTE>(addr4 + i);
        printf("[PATCH] Backup4: %02X %02X %02X %02X %02X %02X\n", origBytes4[0], origBytes4[1], origBytes4[2], origBytes4[3], origBytes4[4], origBytes4[5]);
        DWORD dummy4; VirtualProtect((LPVOID)addr4, 6, old4, &dummy4);
    }
    else {
        printf("[PATCH] Backup4 protect failed: %d\n", GetLastError());
        return;
    }

    NopBytes(addr4, 6);
    isTeamPatched = true;
    printf("[PATCH] Glow patches NOPed successfully\n");

}

void RestoreGlowPatches() {
    if (!isPatched || !Memory::clientDll) {
        printf("[PATCH] Not patched or invalid clientDll (0x%08X)\n", (DWORD)Memory::clientDll);
        return;
    }

    DWORD base = (DWORD)Memory::clientDll;
    DWORD addr1 = base + 0x3DE269;
    DWORD addr2 = base + 0x3DE2CF;
    DWORD addr3 = base + 0x3DE3B7;
    DWORD addr4 = base + 0x3DE2DA;

    printf("[PATCH] Starting restore: base=0x%08X\n", base);

    DWORD old1;
    if (VirtualProtect((LPVOID)addr1, 4, PAGE_EXECUTE_READWRITE, &old1)) {
        for (int i = 0; i < 4; ++i) {
            Memory::Write<BYTE>(addr1 + i, origBytes1[i]);
        }
        printf("[PATCH] Restored1: %02X %02X %02X %02X\n", origBytes1[0], origBytes1[1], origBytes1[2], origBytes1[3]);
        DWORD dummy1; VirtualProtect((LPVOID)addr1, 4, old1, &dummy1);
    }
    else {
        printf("[PATCH] Restore1 protect failed: %d\n", GetLastError());
        return;
    }

    DWORD old2;
    if (VirtualProtect((LPVOID)addr2, 6, PAGE_EXECUTE_READWRITE, &old2)) {
        for (int i = 0; i < 6; ++i) {
            Memory::Write<BYTE>(addr2 + i, origBytes2[i]);
        }
        printf("[PATCH] Restored2: %02X %02X %02X %02X %02X %02X\n", origBytes2[0], origBytes2[1], origBytes2[2], origBytes2[3], origBytes2[4], origBytes2[5]);
        DWORD dummy2; VirtualProtect((LPVOID)addr2, 6, old2, &dummy2);
    }
    else {
        printf("[PATCH] Restore2 protect failed: %d\n", GetLastError());
        return;
    }

    DWORD old3;
    if (VirtualProtect((LPVOID)addr3, 6, PAGE_EXECUTE_READWRITE, &old3)) {
        for (int i = 0; i < 6; ++i) {
            Memory::Write<BYTE>(addr3 + i, origBytes3[i]);
        }
        printf("[PATCH] Restored3: %02X %02X %02X %02X %02X %02X\n", origBytes3[0], origBytes3[1], origBytes3[2], origBytes3[3], origBytes3[4], origBytes3[5]);
        DWORD dummy3; VirtualProtect((LPVOID)addr3, 6, old3, &dummy3);
    }
    else {
        printf("[PATCH] Restore3 protect failed: %d\n", GetLastError());
        return;
    }

    DWORD old4;
    if (VirtualProtect((LPVOID)addr4, 6, PAGE_EXECUTE_READWRITE, &old4)) {
        for (int i = 0; i < 6; ++i) {
            Memory::Write<BYTE>(addr4 + i, origBytes4[i]);
        }
        printf("[PATCH] Restored4: %02X %02X %02X %02X %02X %02X\n", origBytes4[0], origBytes4[1], origBytes4[2], origBytes4[3], origBytes4[4], origBytes4[5]);
        DWORD dummy4; VirtualProtect((LPVOID)addr4, 6, old4, &dummy4);
    }
    else {
        printf("[PATCH] Restore4 protect failed: %d\n", GetLastError());
        return;
    }

    isPatched = false;
    printf("[PATCH] Glow patches restored successfully\n");
}

void RestoreTeamGlowPatches() {
    if (!isTeamPatched || !Memory::clientDll) {
        printf("[PATCH] Not patched or invalid clientDll (0x%08X)\n", (DWORD)Memory::clientDll);
        return;
    }

    DWORD base = (DWORD)Memory::clientDll;
    DWORD addr4 = base + 0x3DE2DA;

    printf("[PATCH] Starting restore: base=0x%08X\n", base);

    DWORD old4;
    if (VirtualProtect((LPVOID)addr4, 6, PAGE_EXECUTE_READWRITE, &old4)) {
        for (int i = 0; i < 6; ++i) {
            Memory::Write<BYTE>(addr4 + i, origBytes4[i]);
        }
        printf("[PATCH] Restored4: %02X %02X %02X %02X %02X %02X\n", origBytes4[0], origBytes4[1], origBytes4[2], origBytes4[3], origBytes4[4], origBytes4[5]);
        DWORD dummy4; VirtualProtect((LPVOID)addr4, 6, old4, &dummy4);
    }
    else {
        printf("[PATCH] Restore4 protect failed: %d\n", GetLastError());
        return;
    }

    isTeamPatched = false;
    printf("[PATCH] Glow patches restored successfully\n");
}