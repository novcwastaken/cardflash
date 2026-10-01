#include "../state.hh"

#include "imgui.h"

namespace CardflashUI {
    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::Begin("Card List", nullptr, flags);

        static int selected = -1;
        const std::vector<Cardflash::Card>& refCards = this->tracked_set.value()->GetRefCards();

        if (ImGui::BeginTable("Cards", 2)) {
            ImGui::TableSetupColumn("Front");
            ImGui::TableSetupColumn("Back");
            ImGui::TableHeadersRow();

            for (int i = 0; i < refCards.size(); i++) {
                ImGui::PushID(i + 1);

                // Front
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                char frontBuf[512];
                strcpy(frontBuf, refCards[i].GetFront().c_str());

                ImGui::InputTextWithHint("##front", "Front...", frontBuf, 255);

                // Back
                ImGui::TableNextColumn();

                char backBuf[512];
                strcpy(backBuf, refCards[i].GetBack().c_str());

                ImGui::InputTextWithHint("##back", "Back...", backBuf, 255);

                ImGui::PopID();
            }

            ImGui::EndTable();
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