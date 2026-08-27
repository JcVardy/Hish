#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class AppMenuBar : public juce::MenuBarModel
{
public:
    enum MenuIds
    {
        fileMenu = 0
    };

    enum FileMenuItems
    {
        openFile = 1
    };

    AppMenuBar();

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    std::function<void()> onOpenFileSelected;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppMenuBar)
};