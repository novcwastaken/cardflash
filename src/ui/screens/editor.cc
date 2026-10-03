#include "../state.hh"

#include "imgui.h"

#include <cstring>
#include <iostream>

static bool SHOW_NO_CARDS_POPUP = false;

namespace CardflashUI {
    void NoCardsPopup() {
        if (SHOW_NO_CARDS_POPUP) ImGui::OpenPopup("No cards!");

        // TODO: Make the popup not resizable
        auto flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        if (ImGui::BeginPopupModal("No cards!", NULL, flags)) {
            ImGui::Text(
                "There must be at least 1 card, and no "
                "card can have empty front or back side"
            );
            if (ImGui::Button("Sure thing...")) {
                SHOW_NO_CARDS_POPUP = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        // If no traked value is set then prompt the user to
        // create a new one
        if (!this->tracked_set.has_value()) {
            // std::cout << "Should open popup" << std::endl;

            static char title[512] = "";
            static char author[512] = "";
            static char subject[512] = "";

            ImGui::OpenPopup("Create New Set");

            // Popup declaration
            if (ImGui::BeginPopupModal("Create New Set", NULL)) {
                ImGui::InputTextWithHint(
                    "##newset_popup_title",
                    "Title (required)",
                    &(title[0]),
                    512
                );
                ImGui::InputTextWithHint(
                    "##newset_popup_author",
                    "Title (required)",
                    &(author[0]),
                    512
                );
                ImGui::InputTextWithHint(
                    "##newset_popup_subject",
                    "Subject (optional)",
                    &(subject[0]),
                    512
                );

                if (
                    ImGui::Button("Create new set")
                    && title[0] != '\0'
                    && author[0] != '\0'
                ) {
                    auto set = Cardflash::Set(
                        std::string(title),
                        std::string(author),
                        std::string(subject)
                    );

                    this->SetSetAsTracked(std::move(set));

                    memset(title,0,sizeof(title));
                    memset(author,0,sizeof(author));
                    memset(subject,0,sizeof(subject));

                    this->editor_front_bufs.clear();
                    this->editor_back_bufs.clear();
                }

                ImGui::EndPopup();
            }
            return;
        }
        assert(this->tracked_set.has_value());
        NoCardsPopup();

        // ==== LEFT SIDE: Card editor ====
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));
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

        if (ImGui::Button("Add card")) {
            this->editor_front_bufs.resize(this->editor_front_bufs.size() + 1);
            this->editor_back_bufs.resize(this->editor_back_bufs.size() + 1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove last card")) {
            this->editor_front_bufs.pop_back();
            this->editor_back_bufs.pop_back();

            // Make sure to not load the cards from the open set
            if (this->editor_front_bufs.empty()) {
                this->editor_front_bufs.resize(1);
                this->editor_back_bufs.resize(1);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save") && !this->SaveTracked()) SHOW_NO_CARDS_POPUP = true;

        ImGui::SameLine();
        if (ImGui::Button("To overview (exit editor)")) {
            if (this->SaveTracked()) this->screen = Screen::SetView;
            else SHOW_NO_CARDS_POPUP = true;
        }

        if (ImGui::BeginTable("Cards", 3)) {
            ImGui::TableSetupColumn("Front###editortablefrontcolumthing");
            ImGui::TableSetupColumn("Back###editortablebackcolumthing");
            ImGui::TableSetupColumn("Controls###editortablecontrolcolumthing");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < this->editor_front_bufs.size(); i++) {
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

                ImGui::TableNextColumn();
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

                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
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

        ImGui::SetNextWindowPos(ImVec2(window_width*0.8, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.2, window_height));
        ImGui::Begin("Metadata", nullptr, flags);

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

        ImGui::End();
    }
}
