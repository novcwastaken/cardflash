#include "../state.hh"

#include "SDL3/SDL_time.h"
#include "imgui.h"
#include "screens.hh"

#include <format>
#include <string>

using namespace Cardflash;
using namespace CardflashUI;

namespace CardflashUI {
    /// The main menu (wow)
    void UiState::MainMenu(int window_width, int window_height, int top_bar_height) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height)); // Magic number
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height));

        ImGui::Begin("Main Menu", nullptr, flags);

        ImGui::Text("Welcome back!");

        if (ImGui::Button("Create")) {}
        ImGui::SetItemTooltip("Create a brand new set.");

        ImGui::SameLine();
        if (ImGui::Button("Search")) {}
        ImGui::SetItemTooltip("Open a searchbar to look through loaded cards.");

        ImGui::SameLine();
        if (ImGui::Button("Import")) {}
        ImGui::SetItemTooltip("Open a file dialog a import a set from the system.");

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
            ImGui::SetItemTooltip("Scan the user directory of cardflash for any new card sets.");
        }

        // Recents
        if (ImGui::BeginTable("main_menu_recents_table", 5)) {
            // Header
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Author");
            ImGui::TableSetupColumn("Subject");
            ImGui::TableSetupColumn("Cards");
            ImGui::TableSetupColumn("Last opened");

            ImGui::TableHeadersRow();

            // Rows
            void UpdateOrderedSets();
            for (const Set& s : this->ordered_sets) {
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
                    last_opened_fmt
                );
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void RecentSetTableRow(std::string name, std::string author, std::string subject, int questions, std::string last_opened) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%s", name.c_str());

        ImGui::TableNextColumn();
        ImGui::Text("%s", author.c_str());

        ImGui::TableNextColumn();
        ImGui::Text("%s", subject.c_str());

        ImGui::TableNextColumn();
        ImGui::Text("%d", questions);

        ImGui::TableNextColumn();
        ImGui::Text("%s", last_opened.c_str());
    }
}