#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class WaveformView : public juce::Component,
                      private juce::ChangeListener
{
public:
    explicit WaveformView(juce::AudioFormatManager& formatManagerToUse);
    ~WaveformView() override;

    void setFile(const juce::File& file);
    void clear();
    void setPlayheadPosition(double positionInSeconds);
    void setNumChannels(int newNumChannels);
    void setVisibleRange(double startSeconds, double endSeconds);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    std::function<void(double)> onSeek;
    std::function<void(double mouseXProportion, float wheelDeltaY)> onZoom;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    juce::Rectangle<int> getChannelBand(int channelIndex, juce::Rectangle<int> bounds) const;

    juce::AudioThumbnailCache thumbnailCache { 5 };
    juce::AudioThumbnail thumbnail;
    bool hasFileLoaded = false;
    double playheadPosition = 0.0;
    int numChannels = 1;

    double visibleStart = 0.0;
    double visibleEnd = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformView)
};