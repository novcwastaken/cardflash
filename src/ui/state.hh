// The state of the UI

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