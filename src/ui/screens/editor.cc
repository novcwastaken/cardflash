#include "../state.hh"

#include "imgui.h"

#include <cstring>
#include <iostream>

static bool SHOW_NO_CARDS_POPUP = false;

namespace CardflashUI {
    void UiState::NoCardsPopup() {
        if (SHOW_NO_CARDS_POPUP) ImGui::OpenPopup("No cards!");

        auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::BeginPopupModal("No cards!", NULL, flags)) {
            ImGui::Text(
                "There must be at least 1 card in a set,\n"
                "with no card having an empty front or\n"
                "back! The title and author fields also\n"
                "mustn't be empty.\n\n"
            );
            if (ButtonWrapper("OK")) {
                SHOW_NO_CARDS_POPUP = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UiState::CreateNewSet() {
        static std::array<char, 1024> title;
        static std::array<char, 1024> author;
        static std::array<char, 1024> subject;

        auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowSize(ImVec2(300, 150));
        if (ImGui::BeginPopupModal("Create New Set", NULL, flags)) {
            ImGui::PushItemWidth(280);
            ImGui::InputTextWithHint(
                "##newset_popup_title",
                "Title*",
                &(title[0]),
                1024
            );
            ImGui::InputTextWithHint(
                "##newset_popup_author",
                "Author*",
                &(author[0]),
                1024
            );
            ImGui::InputTextWithHint(
                "##newset_popup_subject",
                "Subject",
                &(subject[0]),
                1024
            );
            ImGui::PopItemWidth();

            if (ButtonWrapper("OK")
                && title[0] != '\0'
                && author[0] != '\0'
            ) {
                auto set = Cardflash::Set(
                    std::string(&title.front()),
                    std::string(&author.front()),
                    std::string(&subject.front())
                );

                this->editor_title = std::move(title);
                this->editor_author = std::move(author);
                this->editor_subject = std::move(subject);
                title.fill(0);
                author.fill(0);
                subject.fill(0);

                this->SetSetAsTracked(std::move(set));

                this->editor_front_bufs.clear();
                this->editor_back_bufs.clear();
            }

            ImGui::SameLine();
            if (ButtonWrapper("Cancel")) {
                title.fill(0);
                author.fill(0);
                subject.fill(0);
                this->screen = Screen::Menu;
            }

            ImGui::EndPopup();
        }
    }

    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        float margin = 8;

        if (!this->tracked_set.has_value()) {
            ImGui::OpenPopup("Create New Set");
            this->CreateNewSet();
            return;
        }
        assert(this->tracked_set.has_value());
        NoCardsPopup();

        // ==== LEFT SIDE: Card editor ====
        ImGui::SetNextWindowPos(ImVec2(margin, top_bar_height + margin));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8 - margin*2, window_height - top_bar_height - margin*2));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Card List", nullptr, flags);

        // If the set has cards load that into the buffers
        // if not then dont. This should only happen once!
        if (this->tracked_set.value()->IsSetFinalized()) {
            const std::vector<Cardflash::Card>& ref_cards = this->tracked_set.value()->GetRefCards();
            if (
                !this->editor_front_bufs.size()
                && (ref_cards.size() != this->editor_front_bufs.size()
                    || ref_cards.size() != this->editor_back_bufs.size()
                )
            ) {
                std::cout << "Resized editor buffers!" << std::endl;

                strcpy(this->editor_title.begin(), this->tracked_set.value()->title.c_str());
                strcpy(this->editor_author.begin(), this->tracked_set.value()->author.c_str());
                strcpy(this->editor_subject.begin(), this->tracked_set.value()->subject.c_str());

                this->editor_front_bufs.resize(ref_cards.size());
                this->editor_back_bufs.resize(ref_cards.size());

                for (size_t i = 0; i < ref_cards.size(); ++i) {
                    strcpy(this->editor_front_bufs[i].begin(), ref_cards[i].GetFront().c_str());
                    strcpy(this->editor_back_bufs[i].begin(), ref_cards[i].GetBack().c_str());
                }
            }
        }

        if (ButtonWrapper("Add")) {
            this->editor_front_bufs.resize(this->editor_front_bufs.size() + 1);
            this->editor_back_bufs.resize(this->editor_back_bufs.size() + 1);
        }
        ImGui::SameLine();
        if (!this->editor_front_bufs.empty() && ButtonWrapper("Remove last card")) {

            this->editor_front_bufs.pop_back();
            this->editor_back_bufs.pop_back();

            // Make sure to not load the cards from the open set
            if (this->editor_front_bufs.empty()) {
                this->editor_front_bufs.resize(1);
                this->editor_back_bufs.resize(1);
            }
        }
        ImGui::SameLine();
        if (ButtonWrapper("Save") && !this->SaveTracked()) SHOW_NO_CARDS_POPUP = true;

        ImGui::SameLine();
        if (ButtonWrapper("Exit Editor")) {
            if (this->SaveTracked()) this->screen = Screen::SetView;
            else SHOW_NO_CARDS_POPUP = true;
        }

        ImGui::Dummy(ImVec2(0, 8));

        if (ImGui::BeginTable("Cards", 3)) {
            // == HEADER ==
            auto table_flags = ImGuiTableColumnFlags_WidthFixed;
            ImGui::TableSetupColumn(
                "Front###editortablefrontcolumthing",
                table_flags,
                window_width * 0.8 * 0.4 - margin
            );
            ImGui::TableSetupColumn(
                "Back###editortablebackcolumthing",
                table_flags,
                window_width * 0.8 * 0.4 - margin
            );

            ImGui::TableSetupColumn(
                "Controls###editortablecontrolcolumthing",
                table_flags,
                window_width * 0.8 * 0.2 - margin
            );
            ImGui::TableHeadersRow();

            // == ROWS ==
            for (size_t i = 0; i < this->editor_front_bufs.size(); i++) {
                ImGui::TableNextRow();
                ImGui::PushID(i);

                ImGui::TableNextColumn();
                ImGui::PushItemWidth(window_width * 0.8 * 0.3999);

                //ImGui::PushStyleColor(ImGuiCol_FrameBg, GetCurrentTheme()->base.ToImVec4());

                // Front
                ImGui::InputTextWithHint(
                    "##front",
                    "Front...",
                    this->editor_front_bufs[i].begin(),
                    1024
                );

                // Back
                ImGui::TableNextColumn();

                ImGui::PushItemWidth(window_width * 0.8 * 0.3999);
                ImGui::InputTextWithHint(
                    "##back",
                    "Back...",
                    this->editor_back_bufs[i].begin(),
                    1024
                );

                //ImGui::PopStyleColor();

                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Text, GetCurrentTheme()->crust.ToImVec4());
                if (ImGui::ArrowButton("Up", ImGuiDir_Up) && i > 0) {
                    std::array<char, 1024> previous_front = std::move(this->editor_front_bufs[i - 1]);
                    std::array<char, 1024> previous_back = std::move(this->editor_back_bufs[i - 1]);

                    this->editor_front_bufs[i - 1] = std::move(this->editor_front_bufs[i]);
                    this->editor_back_bufs[i - 1] = std::move(this->editor_back_bufs[i]);

                    this->editor_front_bufs[i] = std::move(previous_front);
                    this->editor_back_bufs[i] = std::move(previous_back);
                }

                ImGui::SameLine();
                if (ImGui::ArrowButton("Down", ImGuiDir_Down) && i != this->editor_front_bufs.size() - 1) {
                    std::array<char, 1024> next_front = std::move(this->editor_front_bufs[i + 1]);
                    std::array<char, 1024> next_back = std::move(this->editor_back_bufs[i + 1]);

                    this->editor_front_bufs[i + 1] = std::move(this->editor_front_bufs[i]);
                    this->editor_back_bufs[i + 1] = std::move(this->editor_back_bufs[i]);

                    this->editor_front_bufs[i] = std::move(next_front);
                    this->editor_back_bufs[i] = std::move(next_back);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();
                if (ButtonWrapper("Delete")) {
                    this->editor_front_bufs.erase(this->editor_front_bufs.begin() + i);
                    this->editor_back_bufs.erase(this->editor_back_bufs.begin() + i);

                    if (this->editor_back_bufs.empty()) {
                        // Make sure there is never 0 cards as that would
                        // trigger copying the tracked objet into these buffers
                        this->editor_front_bufs.resize(1);
                        this->editor_back_bufs.resize(1);
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::End();

        // ==== RIGHT SIDE: Metadata ====

        ImGui::SetNextWindowPos(ImVec2(window_width*0.8, top_bar_height + margin));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.2 - margin, window_height - top_bar_height - margin*2));
        ImGui::Begin("Metadata", nullptr, flags);

        ImGui::PushItemWidth(window_width * 0.18);
        ImGui::InputTextWithHint(
            "###editor_title",
            "Title",
            &(this->editor_title[0]),
            sizeof(this->editor_title)
        );

        ImGui::InputTextWithHint(
            "###editor_author",
            "Author",
            &(this->editor_author[0]),
            sizeof(this->editor_author)
        );

        ImGui::InputTextWithHint(
            "###editor_subject",
            "Subject",
            &(this->editor_subject[0]),
            sizeof(this->editor_subject)
        );
        ImGui::PopItemWidth();

        ImGui::End();
    }
}
