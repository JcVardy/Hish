#include "FilterResponseView.h"

FilterResponseView::FilterResponseView()
{
    setSize(340, 220);
}

void FilterResponseView::setResponse(const std::vector<float>& newMagnitudesDb,
                                      const std::vector<float>& newPhasesDegrees,
                                      const std::vector<float>& newImpulseResponse)
{
    magnitudesDb = newMagnitudesDb;
    phasesDegrees = newPhasesDegrees;
    impulseResponse = newImpulseResponse;
    hasData = true;
    repaint();
}

void FilterResponseView::clear()
{
    hasData = false;
    repaint();
}

void FilterResponseView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colours::black);

    if (!hasData)
    {
        g.setColour(juce::Colours::grey);
        g.drawText("Adjust parameters to preview", bounds, juce::Justification::centred);
        return;
    }

    constexpr int labelHeight = 14;
    const int sectionHeight = (bounds.getHeight() - labelHeight * 3) / 3;

    auto magArea = bounds.removeFromTop(labelHeight + sectionHeight);
    auto phaseArea = bounds.removeFromTop(labelHeight + sectionHeight);
    auto impulseArea = bounds;

    g.setColour(juce::Colours::grey);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Magnitude (dB)", magArea.removeFromTop(labelHeight), juce::Justification::left);
    drawMagnitude(g, magArea);

    g.setColour(juce::Colours::grey);
    g.drawText("Phase (degrees)", phaseArea.removeFromTop(labelHeight), juce::Justification::left);
    drawPhase(g, phaseArea);

    g.setColour(juce::Colours::grey);
    g.drawText("Impulse response", impulseArea.removeFromTop(labelHeight), juce::Justification::left);
    drawImpulse(g, impulseArea);
}

void FilterResponseView::drawMagnitude(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(area);

    if (magnitudesDb.size() < 2)
        return;

    constexpr float minDb = -60.0f;
    constexpr float maxDb = 6.0f;

    juce::Path path;

    for (size_t i = 0; i < magnitudesDb.size(); ++i)
    {
        const float x = area.getX() + (static_cast<float>(i) / static_cast<float>(magnitudesDb.size() - 1)) * area.getWidth();
        const float clamped = juce::jlimit(minDb, maxDb, magnitudesDb[i]);
        const float y = area.getBottom() - ((clamped - minDb) / (maxDb - minDb)) * area.getHeight();

        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(juce::Colours::lightgreen);
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

void FilterResponseView::drawPhase(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(area);

    if (phasesDegrees.size() < 2)
        return;

    float minDeg = phasesDegrees[0];
    float maxDeg = phasesDegrees[0];

    for (auto v : phasesDegrees)
    {
        minDeg = juce::jmin(minDeg, v);
        maxDeg = juce::jmax(maxDeg, v);
    }

    if (maxDeg - minDeg < 1.0f)
    {
        minDeg -= 1.0f;
        maxDeg += 1.0f;
    }

    juce::Path path;

    for (size_t i = 0; i < phasesDegrees.size(); ++i)
    {
        const float x = area.getX() + (static_cast<float>(i) / static_cast<float>(phasesDegrees.size() - 1)) * area.getWidth();
        const float y = area.getBottom() - ((phasesDegrees[i] - minDeg) / (maxDeg - minDeg)) * area.getHeight();

        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(juce::Colours::orange);
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

void FilterResponseView::drawImpulse(juce::Graphics& g, juce::Rectangle<int> area) const
{
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(area);

    if (impulseResponse.size() < 2)
        return;

    float peak = 0.0001f;
    for (auto v : impulseResponse)
        peak = juce::jmax(peak, std::abs(v));

    const float centreY = static_cast<float>(area.getCentreY());
    const float halfHeight = area.getHeight() / 2.0f;

    juce::Path path;

    for (size_t i = 0; i < impulseResponse.size(); ++i)
    {
        const float x = area.getX() + (static_cast<float>(i) / static_cast<float>(impulseResponse.size() - 1)) * area.getWidth();
        const float y = centreY - (impulseResponse[i] / peak) * halfHeight;

        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(juce::Colours::skyblue);
    g.strokePath(path, juce::PathStrokeType(1.0f));

    g.setColour(juce::Colours::grey.withAlpha(0.5f));
    g.drawLine(static_cast<float>(area.getX()), centreY, static_cast<float>(area.getRight()), centreY, 0.5f);
}