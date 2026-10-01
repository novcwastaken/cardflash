#include "../state.hh"

#include "imgui.h"
#include <cstring>
#include <iostream>

namespace CardflashUI {
    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());

        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::Begin("Card List", nullptr, flags);

        const std::vector<Cardflash::Card>& ref_cards = this->tracked_set.value()->GetRefCards();
        if (
            ref_cards.size() != this->editor_front_bufs.size()
            || ref_cards.size() != this->editor_back_bufs.size()
        ) {
            std::cout << "Resized!" << std::endl;
            this->editor_front_bufs.resize(ref_cards.size());
            this->editor_back_bufs.resize(ref_cards.size());

            for (size_t i = 0; i < ref_cards.size(); ++i) {
                strcpy(this->editor_front_bufs[i].begin(), ref_cards[i].GetFront().c_str());
                strcpy(this->editor_back_bufs[i].begin(), ref_cards[i].GetBack().c_str());
            }
        }

        if (ImGui::BeginTable("Cards", 2)) {
            ImGui::TableSetupColumn("Front");
            ImGui::TableSetupColumn("Back");
            ImGui::TableHeadersRow();

            for (int i = 0; i < ref_cards.size(); i++) {
                ImGui::PushID(i);
                ImGui::TableNextRow();


                // Front
                ImGui::TableNextColumn();
                ImGui::InputTextWithHint(
                    "##front",
                    "Front...",
                    this->editor_front_bufs[i].begin(),
                    1024
                );

                // Back
                ImGui::TableNextColumn();
                ImGui::InputTextWithHint(
                    "##back",
                    "Back...",
                    this->editor_back_bufs[i].begin(),
                    1024
                );


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