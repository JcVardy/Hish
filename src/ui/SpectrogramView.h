#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include "../core/STFTSettings.h"

class SpectrogramView : public juce::Component,
                         private juce::Thread
{
public:
    explicit SpectrogramView(juce::AudioFormatManager& formatManagerToUse);
    ~SpectrogramView() override;

    void setFile(const juce::File& file);
    void clear();
    void setPlayheadPosition(double positionInSeconds);
    void setSTFTSettings(const STFTSettings& settings);
    void setVisibleRange(double startSeconds, double endSeconds);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

    std::function<void(double)> onSeek;
    std::function<void(double mouseXProportion, float wheelDeltaY)> onZoom;

private:
    void run() override;
    static int orderForSize(int size);

    juce::AudioFormatManager& formatManager;
    juce::File currentFile;
    double fileLengthInSeconds = 0.0;
    double playheadPosition = 0.0;

    double visibleStart = 0.0;
    double visibleEnd = 0.0;

    juce::CriticalSection imageLock;
    juce::Image spectrogramImage;
    bool hasImage = false;

    int fftSize  = 2048;
    int fftOrder = 11;
    int hopSize  = 1024;
    juce::dsp::WindowingFunction<float>::WindowingMethod windowingMethod =
        juce::dsp::WindowingFunction<float>::hann;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramView)
};