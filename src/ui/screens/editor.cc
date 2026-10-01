#include "../state.hh"

#include "imgui.h"

namespace CardflashUI {
    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::Begin("Card List", nullptr, flags);

        if (ImGui::BeginTable("Cards", 2)) {
            ImGui::TableSetupColumn("Front");
            ImGui::TableSetupColumn("Back");
            ImGui::TableHeadersRow();

            const std::vector<Cardflash::Card>& refCards = this->tracked_set.value()->GetRefCards();
            for (int i = 0; i < refCards.size(); ++i)
        }

        ImGui::End();

        // -------------

        ImGui::SetNextWindowPos(ImVec2(window_width*0.8, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.2, window_height));

        ImGui::Begin("Card Inspector", nullptr, flags);

        char front[128] = "";
        ImGui::InputTextWithHint("##01", "Front", front, IM_COUNTOF(front));

        char back[128] = "";
        // ImGui::InputTextWithHint("##02", "Back", back, IM_COUNTOF(back));
        ImGui::InputTextMultiline("##source", back, IM_COUNTOF(back), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), flags);

        ImGui::End();
    }
}