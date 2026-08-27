#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "StatusBar.h"
#include "WaveformView.h"
#include "SpectrogramView.h"
#include "DetailsView.h"
#include "ControlBar.h"
#include "TimeRuler.h"
#include "AmplitudeRuler.h"
#include "FrequencyRuler.h"
#include "../core/AudioEngine.h"
#include "../core/STFTSettings.h"

class MainComponent : public juce::Component,
                       private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    StatusBar& getStatusBar() { return statusBar; }

    void openFile();

private:
    void loadFile(const juce::File& file);
    void timerCallback() override;
    void handleZoom(double mouseXProportion, float wheelDeltaY);
    void updateVisibleRange();

    juce::AudioFormatManager formatManager;
    AudioEngine audioEngine;
    StatusBar statusBar;
    ControlBar controlBar;
    WaveformView waveformView;
    SpectrogramView spectrogramView;
    DetailsView detailsView;
    TimeRuler timeRuler;
    AmplitudeRuler amplitudeRuler;
    FrequencyRuler frequencyRuler;
    std::unique_ptr<juce::FileChooser> fileChooser;

    ControlBar::ViewMode currentViewMode = ControlBar::ViewMode::waveform;

    double totalDuration = 0.0;
    double visibleStart = 0.0;
    double visibleEnd = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};