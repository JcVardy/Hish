#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class NavigationBar : public juce::Component
{
public:
    explicit NavigationBar(bool isVerticalBar);

    juce::ScrollBar& getScrollBar() { return scrollBar; }

    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void()> onZoomInClicked;
    std::function<void()> onZoomOutClicked;

private:
    bool isVertical;

    juce::ScrollBar scrollBar;
    juce::ShapeButton zoomInButton  { "ZoomIn",  juce::Colours::white, juce::Colours::white, juce::Colours::white };
    juce::ShapeButton zoomOutButton { "ZoomOut", juce::Colours::white, juce::Colours::white, juce::Colours::white };

    juce::Rectangle<int> zoomInBoxBounds;
    juce::Rectangle<int> zoomOutBoxBounds;
    int dividerPosition = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NavigationBar)
};