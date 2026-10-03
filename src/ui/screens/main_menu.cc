#include "../state.hh"

#include "SDL3/SDL_time.h"
#include "imgui.h"

#include <format>
#include <string>

using namespace Cardflash;
using namespace CardflashUI;

namespace CardflashUI {
    void RecentSetTableRow(
        std::string name,
        std::string author,
        std::string subject,
        int questions,
        std::string last_opened,
        bool *selected,
        size_t index
    ) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::PushID(1 + index);
        ImGui::Selectable(name.c_str(), selected);
        ImGui::PopID();

        ImGui::TableNextColumn();
        ImGui::PushID(1 + index + questions);
        ImGui::Selectable(author.c_str(), selected);
        ImGui::PopID();

        ImGui::TableNextColumn();
        ImGui::PushID(1 + index + questions * 2);
        ImGui::Selectable(subject.c_str(), selected);
        ImGui::PopID();

        ImGui::TableNextColumn();
        ImGui::PushID(1 + index + questions * 3);
        ImGui::Selectable(std::to_string(questions).c_str(), selected);
        ImGui::PopID();

        ImGui::TableNextColumn();
        ImGui::PushID(1 + index + questions * 4);
        ImGui::Selectable(last_opened.c_str(), selected);
        ImGui::PopID();
    }

    /// The main menu (wow)
    void UiState::MainMenu(int window_width, int window_height, int top_bar_height) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height)); // Magic number
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height));
        ImGui::Begin("Main Menu", nullptr, flags);

        ImGui::Text("Welcome back!");

        if (ImGui::Button("Create new set")) this->screen = Screen::Editor;
        ImGui::SetItemTooltip("Create a new set.");

        ImGui::SameLine();
        if (ImGui::Button("Search")) {}
        ImGui::SetItemTooltip("Open a search bar to find saved sets.");

        ImGui::SameLine();
        if (ImGui::Button("Import")) {}
        ImGui::SetItemTooltip("Open a file dialog to find and\nimport a set from your computer.");

        // Refresh
        if (!this->man.IsScanDisabled()) {
            std::string scan_button_text = "Refresh";
            if (this->man.IsScanning()) {
                scan_button_text = "Pondering...";
            } else if (this->man.DidScanFail()) {
                scan_button_text = "Refresh -- Failed!";
            }

            ImGui::SameLine();
            if (ImGui::Button(scan_button_text.c_str())) {
                this->man.Scan();
            }
            ImGui::SetItemTooltip("Scan for new sets in the local directory.");
        }

        // Recents
        if (ImGui::BeginTable("main_menu_recents_table", 5)) {
            // == Header ==
            auto table_flags = ImGuiTableColumnFlags_WidthFixed;
            ImGui::TableSetupColumn("Name", table_flags);
            ImGui::TableSetupColumn("Author", table_flags);
            ImGui::TableSetupColumn("Subject", table_flags);
            ImGui::TableSetupColumn("Cards", table_flags, window_width * 0.05);
            ImGui::TableSetupColumn("Last opened", table_flags);

            ImGui::TableHeadersRow();

            // Rows
            void UpdateOrderedSets();
            for (size_t i = 0; i < this->ordered_sets.size(); ++i) {
                const Set& s = this->ordered_sets[i];

                auto last_opened = s.GetLastOpenedTimestamp();
                std::string last_opened_fmt;

                if (last_opened != 0) {
                    SDL_DateTime dt;
                    SDL_TimeToDateTime(last_opened, &dt, true);

                    last_opened_fmt = std::format(
                        "{}/{:02d}/{:02d} {:02d}:{:02d}",
                        dt.year,
                        dt.month,
                        dt.day,
                        dt.hour,
                        dt.minute
                    );
                } else {
                    last_opened_fmt = "Never";
                }

                RecentSetTableRow(
                    s.title,
                    s.author,
                    s.subject,
                    s.GetRefCards().size(),
                    last_opened_fmt,
                    &this->ordered_sets_table_selection[i].value,
                    i
                );
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}
