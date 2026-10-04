
#include "../state.hh"
#include "backend/backend.hh"
#include "imgui.h"

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
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height - top_bar_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Flashcard View", nullptr, flags);

        // == CARD ==
        const char* card_button_label = flashcard_view_is_current_card_revealed
            ? current_card.GetBack().c_str()
            : current_card.GetFront().c_str();
        if (ImGui::Button(
            card_button_label,
            ImVec2(
                window_width - 2.0f * ImGui::GetStyle().WindowPadding.x,
                0.9f * (window_height - top_bar_height)
            )
        ))
            flashcard_view_is_current_card_revealed = !flashcard_view_is_current_card_revealed;

        // == CONTROLS ==
        if (ImGui::Button("Reveal"))
            flashcard_view_is_current_card_revealed = !flashcard_view_is_current_card_revealed;

        ImGui::SameLine();
        if (!this->flashcard_view_is_shuffled && ImGui::Button("Shuffle")) this->ShuffleIndexOrder();
        else if (this->flashcard_view_is_shuffled && ImGui::Button("Unshuffle")) {
            this->flashcard_view_is_shuffled = false;
            this->LoadIndexOrder();
            this->flashcard_view_index_order_index = 0;
        }

        ImGui::SameLine();
        if (ImGui::Button("Know")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Know
                );
        }

        ImGui::SameLine();
        if (ImGui::Button("Learning")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Learning
                );
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            this->
                tracked_set
                .value()
                ->SetCardLearningStatus(
                    card_index,
                    Cardflash::CardLearningStatus::Unknown
                );
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(flashcard_view_index_order_index == 0);
        if (ImGui::Button("Prev")) {
            UpdateCurrentCard(this->flashcard_view_index_order_index - 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled( this->flashcard_view_index_order_index
            == this->flashcard_view_index_order.size() - 1
        );
        if (ImGui::Button("Next")) {
            UpdateCurrentCard(this->flashcard_view_index_order_index + 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text(
            "%s",
            std::format(
                "{0}/{1}",
                this->flashcard_view_index_order_index + 1,
                ref_cards.size()
            )
            .c_str()
        );

        ImGui::Text("%s", std::format("Learning status: {0}", current_card.LearningStatusFmt()).c_str());

        ImGui::End();
    }

    void UiState::UpdateCurrentCard(size_t new_index) {
        this->flashcard_view_index_order_index = new_index;
        this->flashcard_view_is_current_card_revealed = false;
    }
}