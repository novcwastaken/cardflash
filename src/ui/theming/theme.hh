#ifndef __THEME_HH_IFNDEF__
#define __THEME_HH_IFNDEF__

#include <cstdint>
#include "imgui.h"

namespace CardflashUI {
    // Helper for storing RGB values.
    struct RGB {
        uint8_t r, g, b;
        constexpr ImVec4 ToImVec4() const;
        ImU32 ToImU32() const;
    };

    // Container for Catppuccin theme color keys.
    // The actual themes are instances of this struct.
    struct CatppuccinTheme {
        RGB base;
        RGB mantle;
        RGB crust;

        RGB text;
        RGB overlay;
        RGB accent;

        RGB green;
        RGB yellow;
        RGB red;
    };

    // Container for all the Catppuccin themes because
    // I couldn't figure out how to pass a variable to
    // state.cc, and now WE, yes WE, are going to use
    // this. Injenir 👍
    struct CatppuccinThemes {
        CatppuccinTheme* current;

        CatppuccinTheme mocha = CatppuccinTheme {
            .base = RGB(30, 30, 46),
            .mantle = RGB(24, 24, 37),
            .crust = RGB(17, 17, 27),

            .text = RGB(205, 214, 244),
            .accent = RGB(203, 166, 247),

            .green = RGB(166, 227, 161),
            .yellow = RGB(249, 226, 175),
            .red = RGB(243, 139, 168)
        };

        CatppuccinTheme latte = CatppuccinTheme {
            .base = RGB(239, 241, 245),
            .mantle = RGB(230, 233, 239),
            .crust = RGB(220, 224, 232),

            .text = RGB(76, 79, 105),
            .accent = RGB(136, 57, 239),

            .green = RGB(64, 160, 43),
            .yellow = RGB(223, 142, 29),
            .red = RGB(210, 15, 57)
        };
    };

    // Sets the ImGui colors from the given CatppuccinTheme.
    void SetupCatppuccinTheme(CatppuccinTheme* t);
}

#endif // __THEME_HH_IFNDEF__