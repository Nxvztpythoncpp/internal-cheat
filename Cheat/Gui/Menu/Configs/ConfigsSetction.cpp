#include "ConfigsSection.h"
#include "../../Fonts/IconsFontAwesome.h"
#include <cstdio>


void Menu::DrawConfigsSection() {
    ImVec2 childSize = ImVec2(620, 70);

    ImGui::BeginChild("ConfigChild", childSize, true, ImGuiWindowFlags_NoScrollbar);

    ImVec2 titlePos = ImGui::GetCursorPos();
    ImGui::Text("Config name");
    ImVec2 titleEnd = ImGui::GetCursorPos();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImVec2 descPos = ImGui::GetCursorPos();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "User and KD");
    ImGui::PopFont();
    ImVec2 descEnd = ImGui::GetCursorPos();

    ImVec2 buttonSize = ImVec2(100, 30);

    float buttonY = titlePos.y + (descEnd.y - titlePos.y - buttonSize.y) / 2.0f;
    ImGui::SetCursorPosX(childSize.x - buttonSize.x - 10);
    ImGui::SetCursorPosY(buttonY);

    if (ImGui::Button("Load " ICON_FA_SERINGE, buttonSize)) {
        printf("Loaded Config\n");
    }

    ImGui::EndChild();
}