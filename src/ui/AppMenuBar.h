#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/ProcessTypes.h"

class AppMenuBar : public juce::MenuBarModel
{
public:
    enum MenuIds
    {
        fileMenu = 0,
        processMenu = 1
    };

    enum FileMenuItems
    {
        openFile = 1,
        exportFile = 2
    };

    enum ProcessMenuItems
    {
        gain = 1,
        normalize = 2,
        firFilter = 3,
        convolution = 4,
        iirFilter = 5
    };

    AppMenuBar();

    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

    std::function<void()> onOpenFileSelected;
    std::function<void()> onExportFileSelected;
    std::function<void(ProcessType)> onProcessSelected;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppMenuBar)
};