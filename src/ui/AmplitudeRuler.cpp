#include "AmplitudeRuler.h"

namespace
{
    constexpr int outerMargin = 14;
    constexpr int interChannelPadding = 10;
}

void AmplitudeRuler::setNumChannels(int newNumChannels)
{
    numChannels = juce::jmax(1, newNumChannels);
    repaint();
}

juce::Rectangle<int> AmplitudeRuler::getChannelBand(int channelIndex, juce::Rectangle<int> bounds) const
{
    const int usableHeight = bounds.getHeight() - (outerMargin * 2) - (interChannelPadding * (numChannels - 1));
    const int bandHeight = juce::jmax(1, usableHeight / numChannels);

    const int bandTop = bounds.getY() + outerMargin + channelIndex * (bandHeight + interChannelPadding);

    return { bounds.getX(), bandTop, bounds.getWidth(), bandHeight };
}

void AmplitudeRuler::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    static const float ticks[] = { 1.0f, 0.0f, -1.0f };

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));

    auto drawArea = bounds.reduced(2);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto band = getChannelBand(ch, drawArea);

        for (float value : ticks)
        {
            const auto y = band.getY() + static_cast<int>(((1.0f - value) / 2.0f) * band.getHeight());

            g.drawLine(static_cast<float>(bounds.getWidth()) - 4.0f, static_cast<float>(y),
                       static_cast<float>(bounds.getWidth()), static_cast<float>(y), 1.0f);
            g.drawText(juce::String(value, 1), 0, y - 7, bounds.getWidth() - 6, 14, juce::Justification::centredRight);
        }
    }
}