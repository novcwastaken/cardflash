// The state of the UI
// It's really not in a good state

#ifndef __STATE_HH_IFNDEF__
#define __STATE_HH_IFNDEF__

#include <GLFW/glfw3.h>

namespace CardflashUI {
    enum class Screen : short;

    class UiState {
        Screen screen;

        /// Renders the top menu bar
        float TopBar(GLFWwindow* window);

        public:
            UiState();
            void Render(int w, int h, GLFWwindow* window);
    };
}

#endif // __STATE_HH_IFNDEF