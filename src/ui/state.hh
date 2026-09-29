// The state of the UI
// It's really not in a good state

#ifndef __STATE_HH_IFNDEF__
#define __STATE_HH_IFNDEF__

#include "backend/backend.hh"
#include <GLFW/glfw3.h>

namespace CardflashUI {
    enum class Screen : short;

    class UiState {
        Screen screen;
        Cardflash::SetManager set_manager = Cardflash::SetManager();

        /// Renders the top menu bar
        float TopBar(GLFWwindow* window);

        /// Renders the main menu
        void MainMenu(int window_width, int window_height, int top_bar_height);
        /// Renders the card editor menu
        void Editor(int window_width, int window_height, int top_bar_height);

        public:
            UiState();
            void Render(int w, int h, GLFWwindow* window);
    };
}

#endif // __STATE_HH_IFNDEF