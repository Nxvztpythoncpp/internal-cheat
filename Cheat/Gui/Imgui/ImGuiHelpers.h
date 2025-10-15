#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <string>
#include <cstdint>

#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#endif
#ifndef ICON_MAX_FA
#define ICON_MAX_FA 0xf957
#endif

// Font icons - keep or adjust
#define ICON_FA_CHART_SIMPLE   u8"\uf080"
#define ICON_FA_CROSSHAIRS     u8"\uf05b"
#define ICON_FA_SHIELD_HALVED  u8"\uf3ed"
#define ICON_FA_EYE            u8"\uf06e"
#define ICON_FA_WAND_MAGIC     u8"\uf0d0"
#define ICON_FA_GEAR           u8"\uf013"
#define ICON_FA_CODE           u8"\uf121"

inline void LoadFontsMergeFA(const char* mainFontPath, float mainFontSize,
    const char* faFontPath, float faSize = 16.0f)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig cfg;
    cfg.OversampleH = cfg.OversampleV = 2;
    cfg.PixelSnapH = false;
    io.Fonts->AddFontFromFileTTF(mainFontPath, mainFontSize, &cfg);

    static const ImWchar ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig merge_cfg;
    merge_cfg.MergeMode = true;
    merge_cfg.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(faFontPath, faSize, &merge_cfg, ranges);
}

inline void AddShadowRect(ImDrawList* dl, const ImRect& r, float rounding,
    ImU32 col, int layers = 6, float spread = 16.0f)
{
    const float alpha = float((col >> IM_COL32_A_SHIFT) & 0xFF);
    const ImU32 rgb = (col & 0x00FFFFFF);

    for (int i = layers; i > 0; --i)
    {
        float t = float(i) / float(layers);
        float grow = spread * t;

        ImVec2 minp(r.Min.x - grow, r.Min.y - grow);
        ImVec2 maxp(r.Max.x + grow, r.Max.y + grow);
        ImRect rr(minp, maxp);

        float a = alpha * (0.06f * t);
        if (a < 1.0f) a = 1.0f;
        ImU32 c = rgb | ((ImU32(a) << IM_COL32_A_SHIFT));

        dl->AddRectFilled(rr.Min, rr.Max, c, rounding + grow);
    }
}

inline bool BeginShadowedChild(const char* str_id, const ImVec2& size,
    float rounding = 12.0f,
    ImU32 border_col = IM_COL32(40, 40, 40, 180),
    ImGuiWindowFlags flags = ImGuiWindowFlags_None)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.06f, 0.09f, 0.95f));

    bool open = ImGui::BeginChild(str_id, size, true, flags);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImGuiWindow* w = ImGui::GetCurrentWindow();
    const ImRect r = w->Rect();

    AddShadowRect(dl, r, rounding, IM_COL32(0, 0, 0, 160), 7, 18.0f);
    dl->AddRect(r.Min, r.Max, border_col, rounding, 0, 1.0f);

    return open;
}

inline void EndShadowedChild()
{
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

// Sidebar button: accepts an icon (font icon or char) + label,
// draws a thin neon left indicator when active and a subtle glow.
inline bool SidebarIconButton(const char* icon, const char* label, bool active, const ImVec2& size)
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.12f, 0.07f, 0.14f, 1.0f) : ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, active ? ImVec4(0.18f, 0.08f, 0.22f, 1.0f) : ImVec4(0.10f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.10f, 0.06f, 0.12f, 1.0f));

    // build label with icon
    std::string lbl;
    if (icon && icon[0] != '\0') {
        lbl = std::string(icon) + "  " + std::string(label);
    }
    else {
        lbl = std::string(label);
    }

    bool pressed = ImGui::Button(lbl.c_str(), size);

    // Draw active indicator
    if (active)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        ImU32 accent = IM_COL32(170, 80, 255, 255); // purple neon
        dl->AddRectFilled(ImVec2(p0.x - 8.0f, p0.y + 4.0f), ImVec2(p0.x - 4.0f, p1.y - 4.0f),
            accent, 4.0f);
        // soft glow
        AddShadowRect(dl, ImRect(ImVec2(p0.x - 10.0f, p0.y + 2.0f), ImVec2(p0.x - 2.0f, p1.y - 2.0f)),
            6.0f, IM_COL32(130, 65, 200, 120), 5, 8.0f);
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    return pressed;
}

// Small modern toggle switch - returns true/false and toggles on click.
// label: optional text on right, and pointer to bool state.
inline bool ToggleSwitch(const char* label, bool* v, float width = 48.0f, float height = 24.0f)
{
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(label, ImVec2(width, height));
    bool hovered = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked())
        *v = !*v;

    // Colors
    ImU32 onCol = IM_COL32(170, 80, 255, 255); // neon purple
    ImU32 offCol = IM_COL32(50, 50, 60, 255);
    ImU32 knobCol = IM_COL32(240, 240, 245, 255);

    float rounding = height * 0.5f;
    float pad = 3.0f;
    ImRect bgRect(p, ImVec2(p.x + width, p.y + height));
    dl->AddRectFilled(bgRect.Min, bgRect.Max, *v ? onCol : offCol, rounding);

    // knob
    float knobMinX = p.x + pad + (*v ? (width - height) : 0.0f);
    float knobMaxX = p.x + pad + (height - pad) + (*v ? (width - height) : 0.0f);
    dl->AddCircleFilled(ImVec2(knobMinX + (knobMaxX - knobMinX) * 0.5f, p.y + height * 0.5f), (height - pad * 2.0f) * 0.5f, knobCol);

    // optional label
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(ImVec2(p.x + width + 8.0f, p.y + (height - ImGui::GetFontSize()) * 0.5f));
    ImGui::TextUnformatted(label);

    return false; // we mutate *v directly
}
