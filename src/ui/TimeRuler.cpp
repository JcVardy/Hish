#include "TimeRuler.h"

void TimeRuler::setRange(double startSeconds, double endSeconds)
{
    rangeStart = startSeconds;
    rangeEnd = endSeconds;
    repaint();
}

juce::String TimeRuler::formatTime(double seconds, bool showDecimal) const
{
    const int minutes = static_cast<int>(seconds) / 60;
    const double secs = seconds - static_cast<double>(minutes) * 60.0;

    if (showDecimal)
        return juce::String::formatted("%d:%04.1f", minutes, secs);

    return juce::String::formatted("%d:%02d", minutes, static_cast<int>(secs));
}

void TimeRuler::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    const double span = rangeEnd - rangeStart;
    if (span <= 0.0)
        return;

    static const double niceSteps[] = { 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 60.0, 120.0, 300.0, 600.0 };

    double step = niceSteps[0];
    for (double candidate : niceSteps)
    {
        step = candidate;
        if (static_cast<int>(span / candidate) <= 12)
            break;
    }

    const bool showDecimal = step < 1.0;

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(11.0f));

    constexpr int labelWidth = 50;
    const double firstTick = std::ceil(rangeStart / step) * step;

    for (double t = firstTick; t <= rangeEnd + 0.0001; t += step)
    {
        const auto x = static_cast<int>(((t - rangeStart) / span) * bounds.getWidth());

        g.drawLine(static_cast<float>(x), 0.0f, static_cast<float>(x), 4.0f, 1.0f);

        const auto textArea = juce::Rectangle<int>(x - labelWidth / 2, 4, labelWidth, bounds.getHeight() - 4);
        g.drawText(formatTime(t, showDecimal), textArea, juce::Justification::centredTop);
    }
}