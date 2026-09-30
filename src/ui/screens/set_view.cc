#include "../state.hh"

#include "backend/backend.hh"
#include "imgui.h"
#include <cassert>

using namespace ImGui;

namespace CardflashUI {
    void UiState::SetView(int window_width, int window_height, int top_bar_height) {
        assert(this->tracked_set.has_value());

        // Fullscreen the window
        SetNextWindowPos(ImVec2(0, top_bar_height));
        SetNextWindowSize(ImVec2(window_width, window_height - top_bar_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        Begin("Card List", nullptr, flags);

        auto style = GetStyle();

        PushFont(NULL, style.FontSizeBase * 4.0f);
        Text("%s", this->tracked_set.value()->title.c_str());
        PopFont();

        Text(
            "%s - %s",
            this->tracked_set.value()->author.c_str(),
            this->tracked_set.value()->subject.c_str()
        );



        End();
    }
}