#pragma once
#include "../Imgui/imgui.h"
#include "../Imgui/ImGuiHelpers.h"

namespace StyleColors {
    inline void Apply()
    {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding = 14.0f;
        s.ChildRounding = 12.0f;
        s.FrameRounding = 10.0f;
        s.GrabRounding = 10.0f;
        s.ScrollbarRounding = 12.0f;

        s.WindowPadding = ImVec2(18, 14);
        s.FramePadding = ImVec2(12, 10);
        s.ItemSpacing = ImVec2(12, 10);
        s.ItemInnerSpacing = ImVec2(10, 8);
        s.IndentSpacing = 18.0f;

        // Base colors
        ImVec4 bg = ImVec4(0.035f, 0.035f, 0.05f, 0.98f);      // deep dark-blue
        ImVec4 card = ImVec4(0.075f, 0.06f, 0.08f, 0.95f);     // slightly purple-tinted card
        ImVec4 text = ImVec4(0.95f, 0.96f, 0.97f, 1.00f);
        ImVec4 dimt = ImVec4(0.72f, 0.72f, 0.78f, 0.95f);
        ImVec4 purple = ImVec4(0.55f, 0.18f, 0.66f, 1.00f);   // main purple
        ImVec4 purpleH = ImVec4(0.75f, 0.28f, 0.86f, 1.00f); // brighter highlight
        ImVec4 lime = ImVec4(0.80f, 1.00f, 0.25f, 1.00f);    // minor accent

        ImGui::PushStyleColor(ImGuiCol_Tab, card); // same as background
        ImGui::PushStyleColor(ImGuiCol_TabHovered, purple);
        ImGui::PushStyleColor(ImGuiCol_TabActive, purpleH);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocused, card);
        ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive, purple);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, bg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, card);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, card);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.12f, 0.22f, 0.7f));

        ImGui::PushStyleColor(ImGuiCol_Text, text);
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, dimt);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.075f, 0.065f, 0.08f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.08f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.12f, 0.09f, 0.14f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_SliderGrab, purple);
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, purpleH);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, lime);

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.08f, 0.06f, 0.09f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.12f, 0.07f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.10f, 0.06f, 0.12f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.20f, 0.12f, 0.22f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.07f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.11f, 0.08f, 0.14f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.09f, 0.07f, 0.12f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotLines, purpleH);
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScrollbarSize = 0.0f;                  
        style.ScrollbarRounding = 0.0f;              
        style.GrabMinSize = 0.0f;                    
        style.GrabRounding = 0.0f;
    }

    inline void Restore() { ImGui::PopStyleColor(27); }
}
