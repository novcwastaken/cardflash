// The state of the UI

// #include "backend/backend.hh"
#include "state.hh"
#include "backend/backend.hh"

namespace CardflashUI {
    void UiState::Render(int w, int h, GLFWwindow* window) {
        this->UpdateOrderedSets();
        float top_bar_height = this->TopBar(window);
        this->MainMenu(w, h, top_bar_height);

        // CardflashUI::Editor(w, h, top_bar_height);
    }

    void UiState::UpdateOrderedSets() {
        if (!this->man.sets_changed)
            return;

        this->ordered_sets = this->man.GetSetsClone();
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