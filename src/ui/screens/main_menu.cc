#include "imgui.h"

namespace CardflashUI {
    /// The main menu (wow)
    void MainMenu() { // TODO: Hide title
        ImGui::Begin("Main Menu");

        if (ImGui::Button("Create")) {}
        ImGui::SameLine();
        if (ImGui::Button("Open")) {}
        ImGui::SameLine();
        if (ImGui::Button("Import")) {}

        ImGui::End();
    }

    /// Empty, undocumented and obsolute function
    void SunshinesAndRainbows() {

    }
}