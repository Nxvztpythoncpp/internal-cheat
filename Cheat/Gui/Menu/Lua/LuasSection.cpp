#include "LuasSection.h"

namespace Menu {

    void DrawLuasSection() {
        ImGui::Text("Luas");
        ImGui::SameLine();
        if (ImGui::SmallButton("Refresh")) {}

        static const char* luas[] = { "Aimbot.lua", "ESP.lua", "Misc.lua" };
        static int selected_lua = -1;
        for (int i = 0; i < IM_ARRAYSIZE(luas); i++) {
            if (ImGui::Selectable(luas[i], selected_lua == i)) {
                selected_lua = i;
            }
        }
    }

}