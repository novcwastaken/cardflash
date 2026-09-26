#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "screens.hh"

namespace CardflashUI {
    void Editor(int window_width, int window_height, int top_bar_height) {
        ImGui::SetNextWindowPos(ImVec2(0, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.8, window_height));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

        ImGui::Begin("Card List", nullptr, flags);
        ImGui::Text("Cards go here");
        ImGui::End();

        // -------------

        ImGui::SetNextWindowPos(ImVec2(window_width*0.8, top_bar_height));
        ImGui::SetNextWindowSize(ImVec2(window_width*0.2, window_height));

        ImGui::Begin("Card Inspector", nullptr, flags);

        char front[128] = "";
        ImGui::InputTextWithHint("##01", "Front", front, IM_COUNTOF(front));

        char back[128] = "";
        ImGui::InputTextWithHint("##02", "Back", back, IM_COUNTOF(back));

        ImGui::End();
    }
}