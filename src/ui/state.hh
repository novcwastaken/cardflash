// The state of the UI
// It's really not in a good state

#ifndef __STATE_HH_IFNDEF__
#define __STATE_HH_IFNDEF__

#include "backend/backend.hh"
#include <GLFW/glfw3.h>
#include <vector>

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

    class UiState {
        Screen screen = Screen::Menu;
        Cardflash::SetManager man = Cardflash::SetManager();
        // An ordered clone of the sets. Needed for the main menu
        // and open recent in the top bar!
        std::vector<Cardflash::Set> ordered_sets;

        void UpdateOrderedSets();

        /// Renders the top menu bar
        float TopBar(GLFWwindow* window);

        /// Renders the main menu
        void MainMenu(int window_width, int window_height, int top_bar_height);
        /// Renders the card editor menu
        void Editor(int window_width, int window_height, int top_bar_height);

        public:
            void Render(int w, int h, GLFWwindow* window);
    };
}

#endif // __STATE_HH_IFNDEF