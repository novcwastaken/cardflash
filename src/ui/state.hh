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

    struct Bool { bool value = false; };

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

        void SetTrackedSet(Cardflash::Set* set);
        void DropTrackedSet();
        std::optional<Cardflash::Set*> tracked_set = std::nullopt;

        // ==== SET VIEW ====
        std::vector<Bool> set_view_is_back_revealed;
        bool set_view_reveal_all = false;
        // ==== !SET VIEW ====

        // ==== EDITOR ====
        void OpenEditor();
        void SetSetAsTracked(Cardflash::Set set);
        // Save tracked_set. Returns false if couldn't save due to
        // front or back buffers being empty, or a text field being empty!
        //
        // Asserts that tracked_set has a value!
        bool SaveTracked();
        std::optional<Cardflash::Set> editor_temp_set = std::nullopt;
        std::vector<std::array<char, 1024>> editor_front_bufs;
        std::vector<std::array<char, 1024>> editor_back_bufs;
        std::array<char, 1024> editor_title;
        std::array<char, 1024> editor_author;
        std::array<char, 1024> editor_subject;
        // ==== !EDITOR ====

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
