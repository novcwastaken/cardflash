#ifndef __SCREENS_HH_IFNDEF__
#define __SCREENS_HH_IFNDEF__

#include <string>

namespace CardflashUI {
    void MainMenu(int window_width, int window_height, int top_bar_height);
    void PlaceholderRecentSetTableRow(std::string name, std::string author, std::string subject, int questions, std::string last_opened);

    void Editor(int window_width, int window_height, int top_bar_height);
}

#endif