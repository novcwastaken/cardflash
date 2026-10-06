#include "theme.hh"
#include "imgui.h"

namespace CardflashUI {
    void SetupCatppuccinTheme(CatppuccinTheme* t) {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_WindowBg] = t->base.ToImVec4();
        colors[ImGuiCol_PopupBg] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBg] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBgActive] = t->crust.ToImVec4();
        colors[ImGuiCol_TitleBgCollapsed]  = t->crust.ToImVec4();

        colors[ImGuiCol_Text] = t->text.ToImVec4();

        colors[ImGuiCol_Button] = t->accent0.ToImVec4();
        colors[ImGuiCol_ButtonHovered] = t->accent1.ToImVec4();
        colors[ImGuiCol_ButtonActive] = t->accent2.ToImVec4();

        colors[ImGuiCol_HeaderHovered] = t->mantle.ToImVec4();
        colors[ImGuiCol_HeaderActive] = t->base.ToImVec4();
        colors[ImGuiCol_Header] = t->surface0.ToImVec4();

        colors[ImGuiCol_MenuBarBg] = t->base.ToImVec4();
        colors[ImGuiCol_FrameBg] = t->mantle.ToImVec4();

        colors[ImGuiCol_TableHeaderBg] = t->crust.ToImVec4();
        colors[ImGuiCol_TableRowBg] = t->mantle.ToImVec4();
        colors[ImGuiCol_TableRowBgAlt] = t->mantle.ToImVec4();

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
        style.PopupRounding = 6;
        style.MenuItemRounding = 6;

        style.CellPadding = ImVec2(16, 2);

    }
}
