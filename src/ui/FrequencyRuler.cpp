#include "FrequencyRuler.h"

void FrequencyRuler::setSampleRate(double newSampleRate)
{
    sampleRate = newSampleRate;
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

    const double nyquist = sampleRate / 2.0;
    if (nyquist <= 0.0)
        return;

    static const double niceSteps[] = { 100.0, 250.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0 };

    double step = niceSteps[0];
    for (double candidate : niceSteps)
    {
        step = candidate;
        if (static_cast<int>(nyquist / candidate) <= 12)
            break;
    }

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));

    for (double freq = 0.0; freq <= nyquist; freq += step)
    {
        const auto y = bounds.getHeight() - static_cast<int>((freq / nyquist) * bounds.getHeight());

        g.drawLine(static_cast<float>(bounds.getWidth()) - 4.0f, static_cast<float>(y),
                   static_cast<float>(bounds.getWidth()), static_cast<float>(y), 1.0f);
        g.drawText(formatFrequency(freq), 0, y - 7, bounds.getWidth() - 6, 14, juce::Justification::centredRight);
    }
}