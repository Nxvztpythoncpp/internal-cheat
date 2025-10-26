#include "Fonts.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <cstdio>

static bool fonts_initialized = false;
static std::vector<ImFont*> loadedFonts;

// Fonts globais
ImFont* iconFont = nullptr;      // FontAwesome
ImFont* csgoIconFont = nullptr;  // CS:GO / CS2 Icons

// Função auxiliar para carregar fontes
ImFont* LoadFont(const std::string& path, float size, const ImWchar* ranges = nullptr, bool merge = false) {
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.PixelSnapH = true;
    config.MergeMode = merge;

    ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, &config, ranges);
    if (font) {
        loadedFonts.push_back(font);
        printf("[Fonts] Fonte carregada com sucesso: %s\n", path.c_str());
    }
    else {
        printf("[Fonts] ERRO ao carregar fonte: %s\n", path.c_str());
    }
    return font;
}

void InitFonts() {
    if (!fonts_initialized) {
        char username[256] = { 0 };
        DWORD size = sizeof(username);
        GetEnvironmentVariableA("USERNAME", username, size);

        std::string base_path = "C:/Users/";
        base_path += username;
        base_path += "/AppData/Local/Microsoft/Windows/Fonts/";

        //FontAwesome
        static const ImWchar fa_ranges[] = { 0xf000, 0xf976, 0 };
        iconFont = LoadFont(base_path + "fa-solid-900.otf", 16.0f, fa_ranges, true);

        //Cs2Icons
        //static const ImWchar cs2_ranges[] = { 0xe000, 0xe0ff, 0 };
        //csgoIconFont = LoadFont(base_path + "cs2_icons.ttf", 16.0f, cs2_ranges, true);

        fonts_initialized = true;
    }
}

// Getters
ImFont* GetIconFont() { return iconFont; }
ImFont* GetCS2IconFont() { return csgoIconFont; }
