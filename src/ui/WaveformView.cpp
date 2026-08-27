#include "WaveformView.h"

namespace
{
    constexpr int outerMargin = 14;
    constexpr int interChannelPadding = 10;
}

WaveformView::WaveformView(juce::AudioFormatManager& formatManagerToUse)
    : thumbnail(512, formatManagerToUse, thumbnailCache)
{
    thumbnail.addChangeListener(this);
}

WaveformView::~WaveformView()
{
    thumbnail.removeChangeListener(this);
}

void WaveformView::setFile(const juce::File& file)
{
    thumbnail.setSource(new juce::FileInputSource(file));
    hasFileLoaded = true;
    playheadPosition = 0.0;
    visibleStart = 0.0;
    visibleEnd = 0.0;
    repaint();
}

void WaveformView::clear()
{
    thumbnail.clear();
    hasFileLoaded = false;
    playheadPosition = 0.0;
    visibleStart = 0.0;
    visibleEnd = 0.0;
    repaint();
}

void WaveformView::setNumChannels(int newNumChannels)
{
    numChannels = juce::jmax(1, newNumChannels);
    repaint();
}

void WaveformView::setVisibleRange(double startSeconds, double endSeconds)
{
    visibleStart = startSeconds;
    visibleEnd = endSeconds;
    repaint();
}

juce::Rectangle<int> WaveformView::getChannelBand(int channelIndex, juce::Rectangle<int> bounds) const
{
    const int usableHeight = bounds.getHeight() - (outerMargin * 2) - (interChannelPadding * (numChannels - 1));
    const int bandHeight = juce::jmax(1, usableHeight / numChannels);

    const int bandTop = bounds.getY() + outerMargin + channelIndex * (bandHeight + interChannelPadding);

    return { bounds.getX(), bandTop, bounds.getWidth(), bandHeight };
}

void WaveformView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds);

    if (hasFileLoaded && thumbnail.getTotalLength() > 0.0)
    {
        const double rangeStart = (visibleEnd > visibleStart) ? visibleStart : 0.0;
        const double rangeEnd   = (visibleEnd > visibleStart) ? visibleEnd   : thumbnail.getTotalLength();
        const double rangeSpan  = juce::jmax(0.0001, rangeEnd - rangeStart);

        auto drawArea = bounds.reduced(2);

        g.setColour(juce::Colours::lightgreen);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto band = getChannelBand(ch, drawArea);
            thumbnail.drawChannel(g, band, rangeStart, rangeEnd, ch, 1.0f);
        }

        if (playheadPosition >= rangeStart && playheadPosition <= rangeEnd)
        {
            const auto proportion = (playheadPosition - rangeStart) / rangeSpan;
            const auto x = bounds.getX() + static_cast<int>(proportion * bounds.getWidth());

            g.setColour(juce::Colours::red);
            g.drawLine(static_cast<float>(x), static_cast<float>(bounds.getY()),
                       static_cast<float>(x), static_cast<float>(bounds.getBottom()), 2.0f);
        }
    }
    else
    {
        g.setColour(juce::Colours::grey);
        g.drawText("No file loaded", bounds, juce::Justification::centred);
    }
}

void WaveformView::resized()
{
}

void WaveformView::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &thumbnail)
        repaint();
}

void WaveformView::setPlayheadPosition(double positionInSeconds)
{
    playheadPosition = positionInSeconds;
    repaint();
}

void WaveformView::mouseDown(const juce::MouseEvent& event)
{
    if (!hasFileLoaded || thumbnail.getTotalLength() <= 0.0)
        return;

    auto bounds = getLocalBounds();

    const double rangeStart = (visibleEnd > visibleStart) ? visibleStart : 0.0;
    const double rangeEnd   = (visibleEnd > visibleStart) ? visibleEnd   : thumbnail.getTotalLength();

    const auto proportion = juce::jlimit(0.0, 1.0,
        (event.x - bounds.getX()) / static_cast<double>(bounds.getWidth()));

    const auto newPosition = rangeStart + proportion * (rangeEnd - rangeStart);

    if (onSeek)
        onSeek(newPosition);
}

void WaveformView::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (!hasFileLoaded)
        return;

    auto bounds = getLocalBounds();
    const auto proportion = juce::jlimit(0.0, 1.0,
        (event.x - bounds.getX()) / static_cast<double>(bounds.getWidth()));

    if (onZoom)
        onZoom(proportion, wheel.deltaY);
}