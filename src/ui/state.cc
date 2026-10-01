// The state of the UI

#include "state.hh"
#include "backend/backend.hh"
#include "uuid_v4.h"
#include <imgui.h>
#include <cassert>

#define gato goto

namespace CardflashUI {
    void UiState::Render(int w, int h, GLFWwindow* window) {
        this->UpdateOrderedSets();

        if (this->screen == Screen::Menu) {
            for (size_t table_i = 0; table_i < this->ordered_sets_table_selection.size(); ++table_i) {
                if (!this->ordered_sets_table_selection[table_i].value) continue;
                this->ordered_sets_table_selection[table_i] = Bool {.value = false};

                UUIDv4::UUID target_uuid = this->ordered_sets[table_i].GetUUID();

                auto setref = this->man.GetSetsRef();
                for (size_t ref_i = 0; ref_i < setref.size(); ++ref_i) {
                    if (setref[ref_i].GetUUID() == target_uuid) {
                        this->tracked_set = this->man.GetSet(ref_i);
                        this->screen = Screen::SetView;
                        gato exitloop;
                    }
                }
            }
        }
        exitloop:

        float top_bar_height = this->TopBar(window);

        switch (this->screen) {
            case Screen::Menu:
                this->MainMenu(w, h, top_bar_height);
                break;
            case Screen::SetView:
                this->SetView(w, h, top_bar_height);
                break;
            case Screen::FlashCard:
                this->FlashCard(w, h, top_bar_height);
                break;
            case Screen::Editor:
                this->Editor(w, h, top_bar_height);
                break;
        }
    }

    void UiState::UpdateOrderedSets() {
        if (!this->man.sets_changed)
            return;


        this->ordered_sets = this->man.GetSetsClone();
        this->ordered_sets_table_selection.clear();
        this->ordered_sets_table_selection.resize(this->ordered_sets.size());

        this->man.sets_changed = false;

        std::sort(
            this->ordered_sets.begin(),
            this->ordered_sets.end(),
            [] (const Cardflash::Set& lhs, const Cardflash::Set& rhs) {
                return
                lhs.GetLastOpenedTimestamp()
                    < rhs.GetLastOpenedTimestamp();
            }
        );
    }

    void UiState::SetTrackedSet(Cardflash::Set* set) {
        // Make sure the tracked set doesn't have a value
        assert(!this->tracked_set.has_value());

        this->tracked_set = set;
    }

    void UiState::DropTrackedSet() {
        assert(this->tracked_set.has_value());

        this->man.DropSetRef(this->tracked_set.value());
        this->tracked_set = std::nullopt;
    }

    void UiState::OpenEditor() {
        if (!this->tracked_set.has_value()) {
            std::cout << "Should open popup" << std::endl;

            static bool is_newset_popup_open = true;
            static char title[512] = "";
            static char author[512] = "";
            static char subject[512] = "";

            while (is_newset_popup_open) {
                if (ImGui::BeginPopupModal("New Set", &is_newset_popup_open)) {
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
                        && title[1] != '\0'
                        && author[1] != '\0'
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

                        is_newset_popup_open = false;
                    }

                    ImGui::EndPopup();
                }
            }
        }

        this->screen = Screen::Editor;
    }

    void UiState::SetSetAsTracked(Cardflash::Set set) {
        assert(!this->tracked_set.has_value());
        assert(!this->editor_temp_set.has_value());

        this->editor_temp_set = std::move(set);
        this->tracked_set = &this->editor_temp_set.value();
    }
}
