#include "theme.hh"
#include "imgui.h"

namespace CardflashUI {
    constexpr ImVec4 RGB::ToImVec4() const {
        return ImVec4((float)this->r/255, (float)this->g/255, (float)this->b/255, 255);
    }

    void SetupCatppuccinTheme(CatppuccinTheme* t) {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_WindowBg] = t->mantle.ToImVec4();
        colors[ImGuiCol_PopupBg] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBg] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBgActive] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBgCollapsed]  = t->crust.ToImVec4();

        colors[ImGuiCol_Text] = t->text.ToImVec4();
        colors[ImGuiCol_Button] = t->accent.ToImVec4(); // temp

        colors[ImGuiCol_MenuBarBg] = t->mantle.ToImVec4();
        colors[ImGuiCol_FrameBg] = t->accent.ToImVec4();

        colors[ImGuiCol_TableRowBg] = t->crust.ToImVec4();

        // Styling
        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding = ImVec2(4, 2);
        style.ItemSpacing = ImVec2(8, 4);
        style.ItemInnerSpacing = ImVec2(4, 4);

        style.WindowBorderSize = 0;
        style.ChildBorderSize = 0;
        style.PopupBorderSize = 0;
        style.FrameBorderSize = 0;

        style.WindowRounding = 8;
        style.ChildRounding = 8;
        style.FrameRounding = 4;
        style.PopupRounding = 4;
        style.MenuItemRounding = 6;
    }
}