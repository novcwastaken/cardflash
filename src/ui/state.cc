// The state of the UI

#include "state.hh"
#include "backend/backend.hh"
#include "uuid_v4.h"

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
}