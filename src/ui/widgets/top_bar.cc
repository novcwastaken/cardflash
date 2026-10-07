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
                if (ImGui::MenuItem("New Set...")) {
                    this->DropTrackedSet();
                    this->screen = Screen::Editor;
                }

                if (ImGui::BeginMenu("Open Recent Set")) {
                    this->UpdateOrderedSets();

                    for (size_t i = 0; i < this->ordered_sets.size(); ++i) {
                        ImGui::PushID(i);
                        if (ImGui::MenuItem(this->ordered_sets[i].title.c_str())) {
                            if (this->tracked_set.has_value())
                                this->DropTrackedSet();

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

                if (this->screen == Screen::Menu) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Import set...")) {
                        this->DropTrackedSet();
                        this->show_import_failure_poup = !this->man.Import();
                    }
                }

                if (this->screen != Screen::Menu && this->tracked_set.has_value()) {
                    ImGui::Separator();
                    if (ImGui::MenuItem("Export Set..."))
                        this->show_export_failure_poup
                            = !this->man.Export(this->tracked_set.value());
                }

                ImGui::Separator();
                if (ImGui::BeginMenu("Themes")) {
                    if (ImGui::MenuItem("Catppuccin Latte (light)"))
                        this->SetCurrentTheme(&(GetThemes()->latte));

                    if (ImGui::MenuItem("Catppuccin Mocha (dark)"))
                        this->SetCurrentTheme(&(GetThemes()->mocha));

                    ImGui::EndMenu();
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Quit")) {
                    glfwSetWindowShouldClose(window, true);
                };

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Main Menu")) {
                // TODO: Not blindly set this if there is any unsaved
                // progress
                if (this->tracked_set.has_value()) {
                    this->DropTrackedSet();
                }
                this->screen = Screen::Menu;
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
