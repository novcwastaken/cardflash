// The state of the UI

#include "state.hh"
#include "ui/screens/screens.hh"
#include "ui/widgets/top_bar.hh"

namespace CardflashUI {
    /// Possible states of the main window
    enum Screen : short {
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
        float top_bar_height = CardflashUI::TopBar(window);
        CardflashUI::MainMenu(w, h, top_bar_height);
    }
}