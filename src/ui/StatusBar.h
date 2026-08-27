#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class StatusBar : public juce::Component
{
public:
    StatusBar();

    void setMessage(const juce::String& message);

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label messageLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusBar)
};