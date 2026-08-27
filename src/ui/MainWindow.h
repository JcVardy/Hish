#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"
#include "AppMenuBar.h"

class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    AppMenuBar menuBarModel;
    MainComponent* mainComponent = nullptr; // DocumentWindow actually owns this

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};