// The state of the UI

// #include "backend/backend.hh"
#include "state.hh"

namespace CardflashUI {
    void UiState::Render(int w, int h, GLFWwindow* window) {
        float top_bar_height = this->TopBar(window);
        this->MainMenu(w, h, top_bar_height);

        // CardflashUI::Editor(w, h, top_bar_height);
    }
}