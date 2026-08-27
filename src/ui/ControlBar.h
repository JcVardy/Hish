#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../core/STFTSettings.h"

class ControlBar : public juce::Component
{
public:
    enum class ViewMode
    {
        waveform,
        spectrogram,
        details
    };

    ControlBar();

    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void()> onPlayClicked;
    std::function<void()> onPauseClicked;
    std::function<void()> onStopClicked;
    std::function<void(ViewMode)> onViewModeChanged;
    std::function<void(const STFTSettings&)> onSTFTSettingsChanged;

private:
    STFTSettings getCurrentSTFTSettings() const;
    void notifySTFTSettingsChanged();

    juce::ShapeButton playButton  { "Play",  juce::Colours::white, juce::Colours::white, juce::Colours::white };
    juce::ShapeButton pauseButton { "Pause", juce::Colours::white, juce::Colours::white, juce::Colours::white };
    juce::ShapeButton stopButton  { "Stop",  juce::Colours::white, juce::Colours::white, juce::Colours::white };

    juce::TextButton waveformViewButton    { "Waveform" };
    juce::TextButton spectrogramViewButton { "Spectrogram" };
    juce::TextButton detailsViewButton     { "Details" };

    juce::Label windowTypeLabel { {}, "Window:" };
    juce::ComboBox windowTypeCombo;

    juce::Label fftSizeLabel { {}, "FFT Size:" };
    juce::ComboBox fftSizeCombo;

    juce::Label overlapLabel { {}, "Overlap:" };
    juce::ComboBox overlapCombo;

    juce::Rectangle<int> playButtonBoxBounds;
    juce::Rectangle<int> pauseButtonBoxBounds;
    juce::Rectangle<int> stopButtonBoxBounds;

    ViewMode currentViewMode = ViewMode::waveform;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ControlBar)
};