#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class AmplitudeRuler : public juce::Component
{
public:
    AmplitudeRuler() = default;
    void setNumChannels(int newNumChannels);
    void paint(juce::Graphics& g) override;

private:
    juce::Rectangle<int> getChannelBand(int channelIndex, juce::Rectangle<int> bounds) const;
    int numChannels = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmplitudeRuler)
};