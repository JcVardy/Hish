#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class FilterResponseView : public juce::Component
{
public:
    FilterResponseView();

    void setResponse(const std::vector<float>& magnitudesDb,
                      const std::vector<float>& phasesDegrees,
                      const std::vector<float>& impulseResponse);
    void clear();

    void paint(juce::Graphics& g) override;

private:
    void drawMagnitude(juce::Graphics& g, juce::Rectangle<int> area) const;
    void drawPhase(juce::Graphics& g, juce::Rectangle<int> area) const;
    void drawImpulse(juce::Graphics& g, juce::Rectangle<int> area) const;

    std::vector<float> magnitudesDb;
    std::vector<float> phasesDegrees;
    std::vector<float> impulseResponse;
    bool hasData = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterResponseView)
};