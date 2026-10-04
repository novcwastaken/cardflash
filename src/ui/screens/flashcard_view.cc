
#include "../state.hh"
#include "backend/backend.hh"
#include "imgui.h"

namespace CardflashUI {
    void UiState::FlashcardView(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());

        std::vector<Cardflash::Card> ref_cards = tracked_set.value()->GetRefCards();

        bool& is_current_card_revealed = this->flashcard_view_is_current_card_revealed;
        size_t& current_card_index = this->flashcard_view_current_card_index;
        Cardflash::Card& current_card = ref_cards[current_card_index];

        // Fullscreen window
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height - top_bar_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Flashcard View", nullptr, flags);

        const char* card_button_label = is_current_card_revealed ? current_card.GetBack().c_str() : current_card.GetFront().c_str();
        if (ImGui::Button(card_button_label, ImVec2(200, 100))) {
            is_current_card_revealed = !is_current_card_revealed;
        };

        if (ImGui::Button("Reveal")) is_current_card_revealed = !is_current_card_revealed;

        ImGui::SameLine();
        ImGui::Button("Shuffle");

        ImGui::SameLine();
        if (ImGui::Button("Know")) {
            current_card.learning_status = Cardflash::CardLearningStatus::Know;
        }

        ImGui::SameLine();
        if (ImGui::Button("Learning")) {
            current_card.learning_status = Cardflash::CardLearningStatus::Learning;
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            current_card.learning_status = Cardflash::CardLearningStatus::Unknown;
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(current_card_index == 0);
        if (ImGui::Button("Prev")) {
            UpdateCurrentCard(current_card_index - 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(current_card_index == ref_cards.size() - 1);
        if (ImGui::Button("Next")) {
            UpdateCurrentCard(current_card_index + 1);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Text("%s", std::format("{0}/{1}", current_card_index + 1, ref_cards.size()).c_str());

        ImGui::Text("%s", std::format("Learning status: {0}", current_card.LearningStatusFmt()).c_str());

        ImGui::End();
    }

    void UiState::UpdateCurrentCard(size_t new_index) {
        this->flashcard_view_current_card_index = new_index;
        this->flashcard_view_is_current_card_revealed = false;
    }
}