#include "ui/state.hh"

#include "imgui.h"
#include <GLFW/glfw3.h>
#include <utility>

namespace CardflashUI {
    /// The menu bar at the top of the window.
    /// Returns the final height of the top bar, due to reasons.
    float UiState::TopBar(GLFWwindow* window) {
        float final_height;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Cardflash")) {
                ImGui::MenuItem("Create Set...");
                ImGui::MenuItem("Open Set...");
                if (ImGui::BeginMenu("Open Recent Set")) { // TODO: Display the titles here instead of the filenames
                    ImGui::MenuItem("sex1.cardflash");
                    ImGui::MenuItem("rape2.cardflash");
                    ImGui::MenuItem("assault3.cardflash");
                    ImGui::MenuItem("anal4.cardflash");
                    ImGui::MenuItem("femboy_thighs5.cardflash");

                    ImGui::EndMenu();
                }

                ImGui::Separator(); // Cool ass sep

                ImGui::MenuItem("Save");
                ImGui::MenuItem("Save As..."); // TODO: Disable (gray out) when the currently opened set is null

                ImGui::Separator();

                ImGui::MenuItem("Import Set...");
                ImGui::MenuItem("Export Set..."); // TODO: Disable (gray out) when the currently opened set is null

                ImGui::Separator();

                ImGui::MenuItem("Preferences");

                ImGui::Separator();

                if (ImGui::MenuItem("Quit")) {
                    glfwSetWindowShouldClose(window, true);
                };

                ImGui::EndMenu();
            }

            final_height = ImGui::GetWindowSize().y;
            ImGui::EndMainMenuBar();
        } else {
            // This branch should not happen
            // Also shuts up the compiler about final_height
            // possibly not being initalized
            std::unreachable();
        }

        return final_height;
    }
}