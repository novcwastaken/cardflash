#include "../state.hh"

#include "backend/backend.hh"
#include "imgui.h"
#include <cassert>

using namespace ImGui;

namespace CardflashUI {
    void UiState::SetView(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());

        // Fullscreen the window
        SetNextWindowPos(ImVec2(0, top_bar_height));
        SetNextWindowSize(ImVec2(window_width, window_height - top_bar_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        Begin("card_view_window", nullptr, flags);

        auto style = GetStyle();

        // ==== Title / author / subject ====
        PushFont(NULL, style.FontSizeBase * 4.0f);
        Text("%s", this->tracked_set.value()->title.c_str());
        PopFont();

        Text(
            "%s - %s",
            this->tracked_set.value()->author.c_str(),
            this->tracked_set.value()->subject.c_str()
        );

        // ==== Stats (TODO!) ====

        // Cirlcle for known/learning/unknown right next to it explaining it

        // ==== Cards ====
        SeparatorText("Cards");

        BeginTable("setview_card_preview", 2);

        // Header
        TableSetupColumn("Front");
        TableSetupColumn("Back (click to reveal)");
        TableHeadersRow();

        const auto& cards = this->tracked_set.value()->GetRefCards();
        if (cards.size() != this->set_view_is_back_revealed.size()) {
            this->set_view_is_back_revealed.resize(cards.size());
        }
        for (size_t i = 0; i < cards.size(); ++i) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::PushID(i);
            ImGui::Selectable(cards[i].GetFront().c_str(), false);
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::PushID(i);
            ImGui::Selectable(
               set_view_is_back_revealed[i].value || set_view_reveal_all
                ? cards[i].GetBack().c_str()
                : "****",
                &this->set_view_is_back_revealed[i].value
            );
            ImGui::PopID();
        }

        EndTable();

        End();
    }
}
