#include "../state.hh"

#include "imgui.h"
#include <cstring>
#include <iostream>

namespace CardflashUI {
    void UiState::Editor(int window_width, int window_height, int top_bar_height) {
        // If no traked value is set then prompt the user to
        // create a new one
        if (!this->tracked_set.has_value()) {
            // std::cout << "Should open popup" << std::endl;

            static char title[512] = "";
            static char author[512] = "";
            static char subject[512] = "";

            ImGui::OpenPopup("Create New Set");

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
                }

                ImGui::EndPopup();
            }
            return;
        }
        assert(this->tracked_set.has_value());

        // ==== LEFT SIDE: Card editor ====

        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Card List", nullptr, flags);

        // If the set has cards load that into the buffers
        // if not then dont
        if (this->tracked_set.value()->IsSetFinalized()) {
            const std::vector<Cardflash::Card>& ref_cards = this->tracked_set.value()->GetRefCards();
            if (
                !this->editor_front_bufs.size()
                && (ref_cards.size() != this->editor_front_bufs.size()
                    || ref_cards.size() != this->editor_back_bufs.size()
                )
            ) {
                std::cout << "Resized!" << std::endl;
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
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (this->editor_front_bufs.empty()) {
                std::cout << "Can't save empty set!" << std::endl;
            }

            this->tracked_set.value()->Clear();
            for (size_t i = 0; i < this->editor_front_bufs.size(); ++i) {
                this->tracked_set
                    .value()
                    ->Expand(
                        Cardflash::Card(
                            std::string(this->editor_front_bufs[i].data()),
                            std::string(this->editor_back_bufs[i].data())
                        )
                    );
            }

            if (this->editor_temp_set.has_value()) {
                this->man.AddSet(std::move(this->editor_temp_set.value()));
                this->editor_temp_set = std::nullopt;

                this->tracked_set = this->man.GetLastSet();
            }

            this->man.Save(this->tracked_set.value());
        }

        if (ImGui::BeginTable("Cards", 2)) {
            ImGui::TableSetupColumn("Front");
            ImGui::TableSetupColumn("Back");
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
