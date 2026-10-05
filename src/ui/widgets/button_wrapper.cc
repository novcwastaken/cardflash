#include "imgui.h"
#include "ui/state.hh"

namespace CardflashUI {
    bool UiState::ButtonWrapper(std::string label, ImVec2 size) {
        ImGui::PushStyleColor(ImGuiCol_Text, this->GetCurrentTheme()->crust.ToImVec4());
        ImGui::PushFont(font_semibold);

        bool meow = ImGui::Button(label.c_str(), size);

        ImGui::PopStyleColor();
        ImGui::PopFont();

        return meow;
    }
}