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
        float margin = 8;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(margin, top_bar_height + margin)); // Magic number
        ImGui::SetNextWindowSize(ImVec2(window_width - margin*2, window_height - top_bar_height - margin*2));

        ImGui::Begin("Main Menu", nullptr, flags);

        // Vertical centering
        static float last_group_height = 0.0f; // Height from previous frame
        float available_height = ImGui::GetContentRegionAvail().y;

        if (last_group_height > 0.0f && available_height > last_group_height) {
            float groupStartY = (available_height - last_group_height) * 0.5f;
            ImGui::SetCursorPosY(groupStartY);
        }

        // Horizontal centering
        float available_width = ImGui::GetContentRegionAvail().x;
        float group_width = available_width / 2;

        float group_start_x = (available_width - group_width) / 2;
        ImGui::SetCursorPosX(group_start_x);

        ImGui::BeginGroup();

        // CARDFLASH
        const char* header1 = "CARDFLASH";
        ImGui::PushFont(font_big);
        float header1_width = ImGui::CalcTextSize(header1).x;
        ImGui::SetCursorPosX(group_start_x + (group_width - header1_width) * 0.5f); // Centered in group
        ImGui::TextUnformatted(header1);
        ImGui::PopFont();

        // Welcome back!
        const char* header2 = "Welcome back!";
        ImGui::PushFont(font_less_bigger_big);
        float header2_width = ImGui::CalcTextSize(header2).x;
        ImGui::SetCursorPosX(group_start_x + (group_width - header2_width) * 0.5f); // Centered in group
        ImGui::TextUnformatted(header2);
        ImGui::PopFont();

        ImGui::Dummy(ImVec2(0, 30));

        // BUTTONS
        const float button_width = 70.0f;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;
        const int button_count = 3; // Create, Import, Refresh

        float total_row_width = (button_width * button_count) + (spacing * (button_count - 1));

        float button_row_start_x = group_start_x + (group_width - total_row_width) * 0.5f;

        ImGui::SetCursorPosX(button_row_start_x);

        if (ButtonWrapper("Create", ImVec2(button_width, 0))) this->screen = Screen::Editor;
        ImGui::SetItemTooltip("Create a new set.");

        // ImGui::SameLine();
        // if (ButtonWrapper("Search", ImVec2(button_width, 0))) {}
        // ImGui::SetItemTooltip("Open a search bar to find saved sets.");

        ImGui::SameLine();
        if (ButtonWrapper("Import", ImVec2(button_width, 0))) {
            this->DropTrackedSet();
            this->show_import_failure_poup = !this->man.Import();
        }
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
            if (ButtonWrapper(scan_button_text.c_str(), ImVec2(button_width, 0))) {
                this->man.Scan();
            }
            ImGui::SetItemTooltip("Scan for new sets in the local directory.");
        }

        ImGui::Dummy(ImVec2(0, 30));

        // Recents
        if (ImGui::BeginTable("main_menu_recents_table", 5, ImGuiTableFlags_RowBg, ImVec2(group_width, 0.0f))) {
            // == Header ==
            auto table_flags = ImGuiTableColumnFlags_WidthFixed;

            ImGui::TableSetupColumn("Name", table_flags);
            ImGui::TableSetupColumn("Author", table_flags);
            ImGui::TableSetupColumn("Subject", table_flags);
            ImGui::TableSetupColumn("Cards", table_flags);
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

        ImGui::EndGroup();
        last_group_height = ImGui::GetItemRectSize().y;

        ImGui::End();
    }
}
