#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class FrequencyRuler : public juce::Component
{
public:
    FrequencyRuler() = default;
    void setRange(double minHz, double maxHz);
    void paint(juce::Graphics& g) override;

private:
    juce::String formatFrequency(double hz) const;
    double rangeMin = 0.0;
    double rangeMax = 22050.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyRuler)
};