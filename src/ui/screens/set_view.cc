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

            if (ButtonWrapper("Yes")) {
                SHOW_EDIT_CONFIRMATION = false;
                this->screen = Screen::Editor;
                ImGui::CloseCurrentPopup();
            }
            SameLine();
            if (ButtonWrapper("I would rather not")) {
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

        // Fullscreen window
        float margin = 8;

        ImGui::SetNextWindowPos(ImVec2(margin, top_bar_height + margin));
        ImGui::SetNextWindowSize(ImVec2(window_width - margin*2, window_height - top_bar_height - margin*2));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        Begin("card_view_window", nullptr, flags);

        Dummy(ImVec2(0, 24));

        // ==== Title / author / subject ====
        std::string title_str = this->tracked_set.value()->title;
        std::string author_str = std::format("by {0}", this->tracked_set.value()->author);
        std::string subject_str = this->tracked_set.value()->subject;

        PushFont(font_big);
        float title_w = CalcTextSize(title_str.c_str()).x;
        PopFont();

        PushFont(font_less_bigger_big);
        float author_w = CalcTextSize(author_str.c_str()).x;
        PopFont();

        PushFont(font_regular);
        float subject_w = CalcTextSize(subject_str.c_str()).x;
        PopFont();

        float text_group_width = std::max({title_w, author_w, subject_w});

        // Circle stuff
        constexpr float circle_diameter = 100.0f;
        constexpr float circle_radius = circle_diameter * 0.5f;
        constexpr float spacing_between_text_and_circle = 30.0f;

        float combined_group_width = text_group_width + spacing_between_text_and_circle + circle_diameter;
        float avail_width = GetContentRegionAvail().x;
        float start_x = (avail_width - combined_group_width) * 0.5f;

        if (start_x > GetCursorPosX()) {
            SetCursorPosX(start_x);
        }

        BeginGroup();

        // --- Left side: Text group ---
        BeginGroup();
        PushFont(font_big);
        TextUnformatted(title_str.c_str());
        PopFont();

        PushFont(font_less_bigger_big);
        TextUnformatted(author_str.c_str());
        PopFont();

        PushFont(font_regular);
        TextUnformatted(subject_str.c_str());
        PopFont();
        EndGroup();



        // ==== Stats (TODO!) ====
        ImDrawList* draw_list = GetWindowDrawList();

        float text_group_height = GetItemRectSize().y;
        float vertical_offset = (text_group_height - circle_diameter) * 0.5f;

        SameLine(0.0f, spacing_between_text_and_circle);
        if (vertical_offset > 0.0f) {
            SetCursorPosY(GetCursorPosY() + vertical_offset);
        }

        ImVec2 p = GetCursorScreenPos();
        ImVec2 center = ImVec2(p.x + circle_radius, p.y + circle_radius);

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
            circle_radius - CIRCLE_LINE_THICKNESS,
            0,
            12
        );
        draw_list->PathStroke(GetThemes()->current->surface0.ToImU32(), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Surface 0

        // std::cout << known_percentage << this->tracked_set.value()->GetRefCards().size() << this->set_view_cards_know << std::endl;
        // Draw the already known circle part with green
        draw_list->PathArcTo(
            center,
            circle_radius - CIRCLE_LINE_THICKNESS,
            RADIAN_OFFSET,
            (2*PI * known_percentage) + RADIAN_OFFSET
        );
        draw_list->PathStroke(GetThemes()->current->green.ToImU32(), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Green

        // Paint the still learning circle to red. Muahaha
        draw_list->PathArcTo(
            center,
            circle_radius - CIRCLE_LINE_THICKNESS,
            // Start where the know circle ends
            (2*PI * known_percentage) + RADIAN_OFFSET,
            (2*PI * known_percentage) + (2*PI * learning_percentage) + RADIAN_OFFSET
        );
        draw_list->PathStroke(GetThemes()->current->red.ToImU32(), CIRCLE_LINE_THICKNESS); // Catppuccin Mocha Red

        Dummy(ImVec2(circle_diameter, circle_diameter));
        SetItemTooltip(
            "This circle displays how many cards\n"
            "you've learned.\n"
            "\n"
            "GREEN - Already know\n"
            "RED - Still learning\n"
            "GRAY - Not specified\n"
            "\n"
            "You can change these tags at any time\n"
            "in the Flashcard View.\n"
        );

        EndGroup();

        Dummy(ImVec2(0, 24));

        // ==== Cards ====
        SeparatorText("Cards");

        if (ButtonWrapper(this->set_view_reveal_all ? "Hide all" : "Reveal all")) {
            this->set_view_reveal_all = !this-> set_view_reveal_all;
        }

        SameLine();
        if (ButtonWrapper("Open Editor")) {
            SHOW_EDIT_CONFIRMATION = true;
        }

        SameLine();
        if (ButtonWrapper("Open Flashcard View")) {
            this->screen = Screen::FlashcardView;
        }

        Dummy(ImVec2(0, 24));

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