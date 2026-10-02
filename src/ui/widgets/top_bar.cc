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
                if (ImGui::MenuItem("Main Menu")) {
                    // TODO: Not blindly set this if there is any unsaved
                    // progress
                    if (this->tracked_set.has_value()) {
                        this->DropTrackedSet();
                    }
                    this->screen = Screen::Menu;
                }

                if (ImGui::MenuItem("New Set...")) this->screen = Screen::Editor;

                if (ImGui::BeginMenu("Open Recent Set")) {
                    this->UpdateOrderedSets();

                    for (size_t i = 0; i < this->ordered_sets.size(); ++i) {
                        ImGui::PushID(i);
                        if (ImGui::MenuItem(this->ordered_sets[i].title.c_str())) {
                            if (this->tracked_set.has_value())
                                this->man.DropSetRef(this->tracked_set.value());
                            UUIDv4::UUID target_uuid = this->ordered_sets[i].GetUUID();

                            auto setref = this->man.GetSetsRef();
                            for (size_t ref_i = 0; ref_i < setref.size(); ++ref_i) {
                                if (setref[ref_i].GetUUID() == target_uuid) {
                                    this->tracked_set = this->man.GetSet(ref_i);
                                    this->screen = Screen::SetView;
                                    break;
                                }
                            }
                        }
                        ImGui::PopID();
                    }

                    ImGui::EndMenu();
                }

                // Not sure if this will actually be implemented, we'll see
                // ImGui::Separator();
                // ImGui::MenuItem("Import Set...");
                // ImGui::MenuItem("Export Set..."); // TODO: Disable (gray out) when the currently opened set is null

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
