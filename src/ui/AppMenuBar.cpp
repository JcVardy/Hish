#include "AppMenuBar.h"

AppMenuBar::AppMenuBar() = default;

juce::StringArray AppMenuBar::getMenuBarNames()
{
    return { "File" };
}

juce::PopupMenu AppMenuBar::getMenuForIndex(int topLevelMenuIndex, const juce::String&)
{
    juce::PopupMenu menu;

    if (topLevelMenuIndex == fileMenu)
    {
        menu.addItem(openFile, "Open...");
    }

    return menu;
}

void AppMenuBar::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    juce::ignoreUnused(menuItemID, topLevelMenuIndex);

    if (menuItemID == openFile && onOpenFileSelected)
        onOpenFileSelected();
}