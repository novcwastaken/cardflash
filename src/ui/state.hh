// The state of the UI
// It's really not in a good state

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