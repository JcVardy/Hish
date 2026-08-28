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
#include "NavigationBar.h"
#include "../core/AudioEngine.h"
#include "../core/STFTSettings.h"
#include "../dsp/ProcessTypes.h"
#include "../dsp/ProcessingEngine.h"
#include "../dsp/FIRFilterProcessor.h"
#include "../dsp/ConvolutionProcessor.h"
#include "../dsp/IIRFilterProcessor.h"
#include "FilterResponseView.h"
#include "../dsp/FilterResponseCalculator.h"

class MainComponent : public juce::Component,
                       private juce::Timer,
                       private juce::ScrollBar::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;

    StatusBar& getStatusBar() { return statusBar; }

    void openFile();
    void exportFile();
    void handleProcessSelected(ProcessType type);

private:
    void loadFile(const juce::File& file, bool addToUndoHistory = true);
    void performUndo();
    void performRedo();
    void timerCallback() override;
    void scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    void handleZoom(double mouseXProportion, float wheelDeltaY);
    void handlePan(double deltaTimeSeconds);
    void handleWaveformVerticalZoom(float wheelDeltaY);
    void handleWaveformVerticalPan(double deltaAmplitude);
    void handleSpectrogramVerticalZoom(double mouseYProportion, float wheelDeltaY);
    void handleSpectrogramVerticalPan(double deltaHz);
    void handleVerticalZoomButton(bool zoomIn);
    void updateVisibleRange();
    void updateScrollBars();

    void applyNormalizeProcess(const juce::File& sourceFile);
    void promptForGain(const juce::File& sourceFile);
    void promptForFIRFilter(const juce::File& sourceFile);
    void promptForConvolution(const juce::File& sourceFile);
    void promptForIIRFilter(const juce::File& sourceFile);

    juce::AudioFormatManager formatManager;
    AudioEngine audioEngine;
    ProcessingEngine processingEngine { formatManager };
    StatusBar statusBar;
    ControlBar controlBar;
    WaveformView waveformView;
    SpectrogramView spectrogramView;
    DetailsView detailsView;
    TimeRuler timeRuler;
    AmplitudeRuler amplitudeRuler;
    FrequencyRuler frequencyRuler;
    NavigationBar horizontalNavBar { false };
    NavigationBar verticalNavBar   { true };
    std::unique_ptr<juce::FileChooser> fileChooser;
    std::vector<juce::File> undoHistory;
    std::vector<juce::File> redoHistory;

    ControlBar::ViewMode currentViewMode = ControlBar::ViewMode::waveform;

    double totalDuration = 0.0;
    double visibleStart = 0.0;
    double visibleEnd = 0.0;

    float waveformVerticalZoom = 1.0f;
    float waveformPanCenter = 0.0f;

    double spectrogramNyquist = 22050.0;
    double spectrogramFreqMin = 0.0;
    double spectrogramFreqMax = 22050.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};