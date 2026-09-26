#include <string>
#include "imgui.h"
#include "screens.hh"

namespace CardflashUI {
    /// The main menu (wow)
    void MainMenu(int window_width, int window_height, int top_bar_height) {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height)); // Magic number
        ImGui::SetNextWindowSize(ImVec2(window_width, window_height));

        ImGui::Begin("Main Menu", nullptr, flags);

        ImGui::Text("Welcome back!");

        if (ImGui::Button("Create")) {}
        ImGui::SameLine();
        if (ImGui::Button("Open")) {}
        ImGui::SameLine();
        if (ImGui::Button("Import")) {}

        // Recents
        if (ImGui::BeginTable("main_menu_recents_table", 5)) {
            // Header
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Author");
            ImGui::TableSetupColumn("Subject");
            ImGui::TableSetupColumn("Questions");
            ImGui::TableSetupColumn("Last opened");

            ImGui::TableHeadersRow();

            // Rows
            CardflashUI::PlaceholderRecentSetTableRow("Hitler Nigger", "Günter", "History", 20, "2026/04/12");

            ImGui::EndTable();
        }

        ImGui::End();
    }

    void PlaceholderRecentSetTableRow(std::string name, std::string author, std::string subject, int questions, std::string last_opened) {
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