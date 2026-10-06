
#include "../state.hh"
#include "backend/backend.hh"

#include <imgui.h>

namespace CardflashUI {
    void UiState::FlashcardView(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());

        if (this->flashcard_view_index_order.empty()) {
            this->LoadIndexOrder();
        }

        const std::vector<Cardflash::Card>& ref_cards = tracked_set.value()->GetRefCards();
        size_t card_index = flashcard_view_index_order[flashcard_view_index_order_index];
        const Cardflash::Card& current_card = ref_cards[card_index];

        // Fullscreen window
        float margin = 8;

        ImGui::SetNextWindowPos(ImVec2(margin, top_bar_height + margin));
        ImGui::SetNextWindowSize(ImVec2(window_width - margin*2, window_height - top_bar_height - margin*2));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Flashcard View", nullptr, flags);

        if (ButtonWrapper("Back to Set View")) {
            this->flashcard_view_is_shuffled = false;
            this->flashcard_view_is_current_card_revealed = false;
            this->flashcard_view_index_order_index = 0;
            this->flashcard_view_index_order.clear();
            this->tracked_set.value()->SetLastOpenedTimestamp();
            this->set_view_should_update_statistics = true;
            this->man.Save(this->tracked_set.value());
            this->man.sets_changed = true;

            this->screen = Screen::SetView;
            ImGui::End();
            return;
        }

        // == CARD ==
        const char* card_button_label = flashcard_view_is_current_card_revealed
            ? current_card.GetBack().c_str()
            : current_card.GetFront().c_str();

        ImGui::PushStyleColor(ImGuiCol_Button, GetCurrentTheme()->mantle.ToImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GetCurrentTheme()->surface0.ToImVec4());
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, GetCurrentTheme()->crust.ToImVec4());
        ImGui::PushFont(font_big);

        if (ImGui::Button(
            card_button_label,
            ImVec2(
                window_width - 2.0f * ImGui::GetStyle().WindowPadding.x - margin * 2,
                0.9f * (window_height - top_bar_height) - 20 - (margin * 2)
            )
        ))
            flashcard_view_is_current_card_revealed = !flashcard_view_is_current_card_revealed;

        ImGui::PopFont();
        ImGui::PopStyleColor(3);



        // == CONTROLS ==
        float available_width = ImGui::GetContentRegionAvail().x;
        float group_width = available_width / 2;

        float group_start_x = (available_width - group_width) / 2;

        const float button_width = 70.0f;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const int button_count = 7;

        float total_row_width = (button_width * button_count) + (spacing * (button_count - 1));

        float button_row_start_x = group_start_x + (group_width - total_row_width) * 0.5f;

        ImGui::SetCursorPosX(button_row_start_x);

        ImGui::BeginGroup();

        if (!flashcard_view_hide_known && ButtonWrapper("Filter known")) {
            this->flashcard_view_hide_known = true;
            this->flashcard_view_index_order_index = 0;

            if (this->flashcard_view_is_shuffled) this->ShuffleIndexOrder();
            else this->LoadIndexOrder(true);
        } else if (flashcard_view_hide_known && ButtonWrapper("Show known")) {
            this->flashcard_view_hide_known = false;
            this->flashcard_view_index_order_index = 0;

            if (this->flashcard_view_is_shuffled) this->ShuffleIndexOrder();
            else this->LoadIndexOrder();
        }

        ImGui::SameLine();
        if (!this->flashcard_view_is_shuffled && ButtonWrapper("Shuffle")) this->ShuffleIndexOrder();
        else if (this->flashcard_view_is_shuffled && ButtonWrapper("Unshuffle")) {
            this->flashcard_view_is_shuffled = false;
            this->LoadIndexOrder(this->flashcard_view_hide_known);
            this->flashcard_view_index_order_index = 0;
        }

        ImGui::SameLine(0, 40);
        if (ButtonWrapper("Know")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Know
                );
        }

        ImGui::SameLine();
        if (ButtonWrapper("Learning")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Learning
                );
        }

        ImGui::SameLine();
        if (ButtonWrapper("Reset")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Unknown
                );
        }

        ImGui::SameLine(0, 40);
        ImGui::BeginDisabled(flashcard_view_index_order_index == 0);
        if (ButtonWrapper("Prev")) {
            UpdateCurrentCard(this->flashcard_view_index_order_index - 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled( this->flashcard_view_index_order_index
            == this->flashcard_view_index_order.size() - 1
        );
        if (ButtonWrapper("Next")) {
            UpdateCurrentCard(this->flashcard_view_index_order_index + 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text(
            "%s",
            std::format(
                "{0}/{1}",
                this->flashcard_view_index_order_index + 1,
                this->flashcard_view_index_order.size()
            )
            .c_str()
        );

        //ImGui::Text("%s", std::format("Learning status: {0}", current_card.LearningStatusFmt()).c_str());
        std::string status_text = std::format("Learning status: {0}", current_card.LearningStatusFmt());
        float status_text_width = ImGui::CalcTextSize(status_text.c_str()).x;
        float status_start_x = (available_width - status_text_width) * 0.5f;

        if (status_start_x > 0.0f) {
            ImGui::SetCursorPosX(status_start_x);
        }
        ImGui::TextUnformatted(status_text.c_str());

        ImGui::EndGroup();

        ImGui::End();
    }

    void UiState::UpdateCurrentCard(size_t new_index) {
        this->flashcard_view_index_order_index = new_index;
        this->flashcard_view_is_current_card_revealed = false;
    }
}