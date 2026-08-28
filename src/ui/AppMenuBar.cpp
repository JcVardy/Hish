#include "AppMenuBar.h"

AppMenuBar::AppMenuBar() = default;

juce::StringArray AppMenuBar::getMenuBarNames()
{
    return { "File", "Process" };
}

juce::PopupMenu AppMenuBar::getMenuForIndex(int topLevelMenuIndex, const juce::String&)
{
    juce::PopupMenu menu;

    if (topLevelMenuIndex == fileMenu)
    {
        menu.addItem(openFile, "Open...");
        menu.addItem(exportFile, "Export...");
    }
    else if (topLevelMenuIndex == processMenu)
    {
        menu.addItem(gain, "Apply Gain...");
        menu.addItem(normalize, "Normalize");
        menu.addItem(firFilter, "FIR Filter");
        menu.addItem(convolution, "Convolution");
        menu.addItem(iirFilter, "IIR Filter");
    }

    return menu;
}

void AppMenuBar::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == fileMenu)
    {
        if (menuItemID == openFile && onOpenFileSelected)
            onOpenFileSelected();
        else if (menuItemID == exportFile && onExportFileSelected)
            onExportFileSelected();
    }
    else if (topLevelMenuIndex == processMenu)
    {
        if (!onProcessSelected)
            return;

        switch (menuItemID)
        {
            case gain:         onProcessSelected(ProcessType::gain);         break;
            case normalize:    onProcessSelected(ProcessType::normalize);    break;
            case firFilter:    onProcessSelected(ProcessType::firFilter);    break;
            case convolution:  onProcessSelected(ProcessType::convolution);  break;
            case iirFilter:    onProcessSelected(ProcessType::iirFilter);    break;
            default: break;
        }
    }
}