// The state of the UI

#ifndef __STATE_HPP_IFNDEF__
#define __STATE_HPP_IFNDEF__

namespace CardflashUI {
    enum Screen : short;

    class UiState {
        Screen screen;

        public:
            UiState();
            void Render(int w, int h);
    };
}

#endif