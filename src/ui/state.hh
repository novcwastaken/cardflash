// The state of the UI
// It's really not in a good state

#ifndef __STATE_HH_IFNDEF__
#define __STATE_HH_IFNDEF__

#include "backend/backend.hh"
#include <GLFW/glfw3.h>
#include <optional>
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

    struct Bool { bool value; };

    class UiState {
        Screen screen = Screen::Menu;
        Cardflash::SetManager man = Cardflash::SetManager();

        // ==== MAIN MENU ====
        // An ordered clone of the sets. Needed for the main menu
        // and open recent in the top bar!
        std::vector<Cardflash::Set> ordered_sets;
        // Refers to an element of ordered_sets (by index) and if any
        // of the elements turn true that set was clicked in the
        // table on the main menu!
        std::vector<Bool> ordered_sets_table_selection;
        void UpdateOrderedSets();
        // ==== !MAIN MENU ====

        std::optional<Cardflash::Set*> tracked_set = std::nullopt;

        /// Renders the top menu bar
        float TopBar(GLFWwindow* window);

        /// Renders the main menu
        void MainMenu(int window_width, int window_height, int top_bar_height);

        /// Renders the card editor menu.
        ///
        /// If tracked set is empty a new set will be created, if tracked set
        /// is not nullopt the object there will be edited.
        void Editor(int window_width, int window_height, int top_bar_height);

        /// Renders the set overview screen.
        ///
        /// Tracked set must not be nullopt when calling this
        void SetView(int window_width, int window_height, int top_bar_height);

        // Tracked set must not be nullopt when calling this
        void FlashCard(int window_width, int window_height, int top_bar_height);

        public:
            void Render(int w, int h, GLFWwindow* window);
    };
}

#endif // __STATE_HH_IFNDEF
