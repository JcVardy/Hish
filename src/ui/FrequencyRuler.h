#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class FrequencyRuler : public juce::Component
{
public:
    FrequencyRuler() = default;
    void setSampleRate(double newSampleRate);
    void paint(juce::Graphics& g) override;

private:
    juce::String formatFrequency(double hz) const;
    double sampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyRuler)
};