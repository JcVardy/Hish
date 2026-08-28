#include "FrequencyRuler.h"

namespace
{
    constexpr int outerMargin = 14;
}

void FrequencyRuler::setRange(double minHz, double maxHz)
{
    rangeMin = minHz;
    rangeMax = maxHz;
    repaint();
}

juce::String FrequencyRuler::formatFrequency(double hz) const
{
    if (hz >= 1000.0)
        return juce::String(hz / 1000.0, 1) + "k";
    return juce::String(static_cast<int>(hz));
}

void FrequencyRuler::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    const double span = rangeMax - rangeMin;
    if (span <= 0.0)
        return;

    auto band = bounds.reduced(2);
    band.removeFromTop(outerMargin);
    band.removeFromBottom(outerMargin);

    static const double niceSteps[] = { 10.0, 20.0, 50.0, 100.0, 250.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0 };

    double step = niceSteps[0];
    for (double candidate : niceSteps)
    {
        step = candidate;
        if (static_cast<int>(span / candidate) <= 12)
            break;
    }

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));

    const double firstTick = std::ceil(rangeMin / step) * step;

    for (double freq = firstTick; freq <= rangeMax + 0.0001; freq += step)
    {
        const auto y = band.getY() + band.getHeight()
                        - static_cast<int>(((freq - rangeMin) / span) * band.getHeight());

        g.drawLine(static_cast<float>(bounds.getWidth()) - 4.0f, static_cast<float>(y),
                   static_cast<float>(bounds.getWidth()), static_cast<float>(y), 1.0f);
        g.drawText(formatFrequency(freq), 0, y - 7, bounds.getWidth() - 6, 14, juce::Justification::centredRight);
    }
}