#include "NavigationBar.h"

namespace
{
    juce::Path makeMagnifierGlassPath()
    {
        juce::Path p;
        p.addEllipse(0.05f, 0.05f, 0.55f, 0.55f);
        p.addEllipse(0.16f, 0.16f, 0.33f, 0.33f);
        p.setUsingNonZeroWinding(false);

        p.addLineSegment(juce::Line<float>(0.5f, 0.5f, 0.95f, 0.95f), 0.12f);
        return p;
    }

    juce::Path makeZoomInPath()
    {
        auto p = makeMagnifierGlassPath();
        p.addLineSegment(juce::Line<float>(0.32f, 0.18f, 0.32f, 0.46f), 0.09f);
        p.addLineSegment(juce::Line<float>(0.18f, 0.32f, 0.46f, 0.32f), 0.09f);
        return p;
    }

    juce::Path makeZoomOutPath()
    {
        auto p = makeMagnifierGlassPath();
        p.addLineSegment(juce::Line<float>(0.18f, 0.32f, 0.46f, 0.32f), 0.09f);
        return p;
    }
}

NavigationBar::NavigationBar(bool isVerticalBar)
    : isVertical(isVerticalBar), scrollBar(isVerticalBar)
{
    auto background = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(0.3f);
    auto thumb = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).brighter(0.4f);

    scrollBar.setColour(juce::ScrollBar::backgroundColourId, background);
    scrollBar.setColour(juce::ScrollBar::thumbColourId, thumb);
    scrollBar.setAutoHide(false);
    addAndMakeVisible(scrollBar);

    zoomInButton.setShape(makeZoomInPath(), true, true, false);
    zoomOutButton.setShape(makeZoomOutPath(), true, true, false);

    zoomInButton.setMouseClickGrabsKeyboardFocus(false);
    zoomOutButton.setMouseClickGrabsKeyboardFocus(false);

    zoomInButton.onClick  = [this] { if (onZoomInClicked)  onZoomInClicked(); };
    zoomOutButton.onClick = [this] { if (onZoomOutClicked) onZoomOutClicked(); };

    addAndMakeVisible(zoomInButton);
    addAndMakeVisible(zoomOutButton);
}

void NavigationBar::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(zoomInBoxBounds.toFloat(), 4.0f);
    g.fillRoundedRectangle(zoomOutBoxBounds.toFloat(), 4.0f);

    g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));

    if (isVertical)
    {
        const float dividerY = static_cast<float>(dividerPosition);
        g.drawLine(0.0f, dividerY, static_cast<float>(getWidth()), dividerY, 1.0f);
    }
    else
    {
        const float dividerX = static_cast<float>(dividerPosition);
        g.drawLine(dividerX, 0.0f, dividerX, static_cast<float>(getHeight()), 1.0f);
    }
}

void NavigationBar::resized()
{
    auto bounds = getLocalBounds();

    constexpr int iconSize = 16;
    constexpr int boxPadding = 6;
    constexpr int spacing = 1;
    constexpr int dividerGap = 3;
    constexpr int buttonsAreaThickness = iconSize + (boxPadding * 2) + 4;

    if (isVertical)
    {
        auto buttonsArea = bounds.removeFromTop(buttonsAreaThickness * 2 + spacing);

        auto inArea  = buttonsArea.removeFromTop(buttonsAreaThickness).withSizeKeepingCentre(iconSize, iconSize);
        buttonsArea.removeFromTop(spacing);
        auto outArea = buttonsArea.removeFromTop(buttonsAreaThickness).withSizeKeepingCentre(iconSize, iconSize);

        zoomInButton.setBounds(inArea);
        zoomOutButton.setBounds(outArea);

        zoomInBoxBounds  = inArea.expanded(boxPadding);
        zoomOutBoxBounds = outArea.expanded(boxPadding);

        bounds.removeFromTop(dividerGap);
        dividerPosition = bounds.getY();
        bounds.removeFromTop(dividerGap);

        scrollBar.setBounds(bounds);
    }
    else
    {
        auto buttonsArea = bounds.removeFromRight(buttonsAreaThickness * 2 + spacing);

        auto outArea = buttonsArea.removeFromLeft(buttonsAreaThickness).withSizeKeepingCentre(iconSize, iconSize);
        buttonsArea.removeFromLeft(spacing);
        auto inArea  = buttonsArea.removeFromLeft(buttonsAreaThickness).withSizeKeepingCentre(iconSize, iconSize);

        zoomOutButton.setBounds(outArea);
        zoomInButton.setBounds(inArea);

        zoomOutBoxBounds = outArea.expanded(boxPadding);
        zoomInBoxBounds  = inArea.expanded(boxPadding);

        bounds.removeFromRight(dividerGap);
        dividerPosition = bounds.getRight();
        bounds.removeFromRight(dividerGap);

        scrollBar.setBounds(bounds);
    }
}