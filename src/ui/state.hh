// The state of the UI
// It's really not in a good state

#ifndef __STATE_HH_IFNDEF__
#define __STATE_HH_IFNDEF__

#include <GLFW/glfw3.h>

namespace CardflashUI {
    enum Screen : short;

    class UiState {
        Screen screen;

        public:
            UiState();
            void Render(int w, int h, GLFWwindow* window);
    };
}

#endif