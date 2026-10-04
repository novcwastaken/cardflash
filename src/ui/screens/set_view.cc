#include "../state.hh"

#include "backend/backend.hh"
#include "imgui.h"
#include <cassert>
#include <numbers>

using namespace ImGui;

// Constants for the custom statistics draw commands
constexpr float CIRCLE_LINE_THICKNESS = 15;
// Ofc cpp would have a type template for PI.
// idfk what constexpr, it sounds cooler
constexpr float PI = std::numbers::pi_v<float>;
constexpr float RADIAN_OFFSET = PI * -0.5f;

static bool SHOW_EDIT_CONFIRMATION = false;

namespace CardflashUI {
    void UiState::SureToEditPopup() {
        if (SHOW_EDIT_CONFIRMATION) ImGui::OpenPopup("suretoedit");

        auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::BeginPopupModal("Are you sure?###suretoedit", NULL, flags)) {
            ImGui::Text(
                "Editing a set will reset all statistics\n"
                "data! Do you still wish to proceed?"
            );

            if (ImGui::Button("Yes")) {
                SHOW_EDIT_CONFIRMATION = false;
                this->screen = Screen::Editor;
                ImGui::CloseCurrentPopup();
            }
            SameLine();
            if (Button("I would rather not")) {
                SHOW_EDIT_CONFIRMATION = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UiState::SetView(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());
        this->GetTrackedSetStatistics();
        this->SureToEditPopup();

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
        ImDrawList* draw_list = GetWindowDrawList();
        ImVec2 p = GetCursorScreenPos();
        ImVec2 center = p;
        center.x += 50;
        center.y += 50;

        // == Cirlcle for known/learning/unknown right next to it explaining it ==
        float known_percentage, learning_percentage;
        // Make sure we aren't dividing by 0
        if (this->set_view_cards_know == 0) known_percentage = 0;
        else known_percentage = this->set_view_cards_know
            / static_cast<float>(this->tracked_set.value()->GetRefCards().size());

        if (this->set_view_cards_learning == 0) learning_percentage = 0;
        else learning_percentage = this->set_view_cards_learning
            / static_cast<float>(this->tracked_set.value()->GetRefCards().size());

        // Draw the full grey circle (for unknown)
        // We're using ArctTo instead of AddCircle because this way we can
        // make it a store. We're also using the fast variant as we're
        // drawing the full circle, the loss of precision is fine.
        draw_list->PathArcToFast(
            center,
            50 - CIRCLE_LINE_THICKNESS,
            0,
            12
        );
        draw_list->PathStroke(IM_COL32(69, 71, 90, 255), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Surface 1

        // std::cout << known_percentage << this->tracked_set.value()->GetRefCards().size() << this->set_view_cards_know << std::endl;
        // Draw the already known circle part with green
        draw_list->PathArcTo(
            center,
            50 - CIRCLE_LINE_THICKNESS,
            RADIAN_OFFSET,
            (2*PI * known_percentage) + RADIAN_OFFSET
        );
        draw_list->PathStroke(IM_COL32(166, 227, 161, 255), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Green

        // Draw the still learning circle part with yellow
        draw_list->PathArcTo(
            center,
            50 - CIRCLE_LINE_THICKNESS,
            // Start where the know circle ends
            (2*PI * known_percentage) + RADIAN_OFFSET,
            (2*PI * known_percentage) + (2*PI * learning_percentage) + RADIAN_OFFSET
        );
        draw_list->PathStroke(IM_COL32(249, 226, 175, 255), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Yellow

        Dummy(ImVec2(100, 100));
        SetItemTooltip(
            "This circle displays how many cards\n"
            "you've learned.\n"
            "\n"
            "GREEN - Already know\n"
            "YELLOW - Still learning\n"
            "GRAY - Not specified\n"
            "\n"
            "You can change these tags at any time\n"
            "in the Flashcard View.\n"
        );

        // ==== Cards ====
        SeparatorText("Cards");

        if (Button("Reveal All")) {
            this-> set_view_reveal_all = !this-> set_view_reveal_all;
        }

        SameLine();
        if (Button("Open Editor")) {
            SHOW_EDIT_CONFIRMATION = true;
        }

        SameLine();
        if (Button("Open Flashcard View")) {
            this->screen = Screen::FlashcardView;
        }

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
            ImGui::PushID(i + 1);
            ImGui::Selectable(cards[i].GetFront().c_str(), false);
            ImGui::PopID();

            ImGui::TableNextColumn();
            ImGui::PushID(i + 1);
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
