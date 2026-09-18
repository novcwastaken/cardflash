#include "state.hh"

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
                ImGui::MenuItem("Create Set...");
                ImGui::MenuItem("Open Set...");
                if (ImGui::BeginMenu("Open Recent Set")) {
                    ImGui::MenuItem("sex1.cardflash");
                    ImGui::MenuItem("rape2.cardflash");
                    ImGui::MenuItem("assault3.cardflash");
                    ImGui::MenuItem("anal4.cardflash");
                    ImGui::MenuItem("femboy_thighs5.cardflash");

                    ImGui::EndMenu();
                }

                ImGui::Separator(); // Cool ass sep

                ImGui::MenuItem("Quit");

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}