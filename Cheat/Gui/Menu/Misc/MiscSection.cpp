#include "MiscSection.h"
#include "../../../Cheats/Misc/UninjectHook.h"
#include"../../../Cheats/Misc/Movement.h"

void Menu::DrawMiscSection() {
    ImGui::Text("Misc Settings");
    static bool& bhop = Movement::BunnyhopEnabled;
    static bool bunnyhop = false;
    static bool radar = false;

    ImGui::Checkbox("Bunny Hop", &bhop);
    ImGui::Checkbox("Perfect Bunnyhop", &bunnyhop);
    ImGui::Checkbox("Radar Hack", &radar);

    if (ImGui::Button("Uninject") && !Unhook::IsCleaning()) {
        Unhook::RequestUnload();
        Unhook::StartSafeCleanupThread();
    }


    if (Unhook::IsCleaning()) {
        ImGui::Text("Cleaning in progress...");
    }


}