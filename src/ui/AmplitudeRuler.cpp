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

void AmplitudeRuler::setRange(float minValue, float maxValue)
{
    rangeMin = minValue;
    rangeMax = maxValue;
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

    const float span = rangeMax - rangeMin;
    if (span <= 0.0f)
        return;

    static const float niceSteps[] = { 0.01f, 0.02f, 0.05f, 0.1f, 0.2f, 0.25f, 0.5f, 1.0f };

    float step = niceSteps[0];
    for (float candidate : niceSteps)
    {
        step = candidate;
        if (static_cast<int>(span / candidate) <= 6)
            break;
    }

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));

    auto drawArea = bounds.reduced(2);
    const float firstTick = std::ceil(rangeMin / step) * step;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto band = getChannelBand(ch, drawArea);

        for (float value = firstTick; value <= rangeMax + 0.0001f; value += step)
        {
            const auto y = band.getY() + static_cast<int>(((rangeMax - value) / span) * band.getHeight());

            g.drawLine(static_cast<float>(bounds.getWidth()) - 4.0f, static_cast<float>(y),
                       static_cast<float>(bounds.getWidth()), static_cast<float>(y), 1.0f);
            g.drawText(juce::String(value, 2), 0, y - 7, bounds.getWidth() - 6, 14,
                       juce::Justification::centredRight);
        }
    }
}