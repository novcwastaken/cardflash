// The state of the UI

// #include "backend/backend.hh"
#include "state.hh"

namespace CardflashUI {
    /// Possible states of the main window
    enum class Screen : short {
        /// In the menu
        Menu,
        /// Viewing a set
        SetView,
        /// Playing flashcard mode in a set
        FlashCard,
        /// In editor
        Editor,
    };


    UiState::UiState() {
        screen = Screen::Menu;
    }

    void UiState::Render(int w, int h, GLFWwindow* window) {
        float top_bar_height = this->TopBar(window);
        this->MainMenu(w, h, top_bar_height);

        // CardflashUI::Editor(w, h, top_bar_height);
    }
}