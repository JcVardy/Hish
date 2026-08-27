#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class TimeRuler : public juce::Component
{
public:
    TimeRuler() = default;
    void setRange(double startSeconds, double endSeconds);
    void paint(juce::Graphics& g) override;

private:
    juce::String formatTime(double seconds, bool showDecimal) const;
    double rangeStart = 0.0;
    double rangeEnd = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeRuler)
};