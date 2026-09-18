#include "state.hpp"

#include "imgui.h"

// The state of the ui

namespace CardflashUI {
    /// Possible states of the main window
    enum Screen : short {
        /// In the menu
        Menu,
        /// Viewing a set
        SetView,
        /// Playing flashcard mode in a set
        FlashCard,
        /// In editor
        Editor,
    };


    UiState::UiState() {
        screen = Screen::Menu;
    }

    void UiState::Render(int w, int h) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Cardflash")) {
                ImGui::MenuItem("New Set...");
                ImGui::MenuItem("Open Set...");
                ImGui::Separator();
                ImGui::MenuItem("Quit");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}