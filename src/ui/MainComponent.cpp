#include "MainComponent.h"

MainComponent::MainComponent()
   : audioEngine(formatManager)
     , waveformView(formatManager)
     , spectrogramView(formatManager)
     , detailsView(formatManager)
{
    formatManager.registerBasicFormats();

    addAndMakeVisible(statusBar);
    addAndMakeVisible(controlBar);
    addAndMakeVisible(waveformView);
    addChildComponent(spectrogramView);
    addChildComponent(detailsView);
    addAndMakeVisible(timeRuler);
    addAndMakeVisible(amplitudeRuler);
    addChildComponent(frequencyRuler);

    controlBar.onPlayClicked  = [this] { audioEngine.play(); };
    controlBar.onPauseClicked = [this] { audioEngine.pause(); };
    controlBar.onStopClicked  = [this] { audioEngine.stop(); };

    controlBar.onViewModeChanged = [this](ControlBar::ViewMode mode)
    {
        currentViewMode = mode;
        waveformView.setVisible(mode == ControlBar::ViewMode::waveform);
        spectrogramView.setVisible(mode == ControlBar::ViewMode::spectrogram);
        detailsView.setVisible(mode == ControlBar::ViewMode::details);
        resized();
    };

    controlBar.onSTFTSettingsChanged = [this](const STFTSettings& settings)
    {
        spectrogramView.setSTFTSettings(settings);
    };

    waveformView.onSeek = [this](double positionInSeconds)
    {
        audioEngine.setPosition(positionInSeconds);
        waveformView.setPlayheadPosition(positionInSeconds);
        spectrogramView.setPlayheadPosition(positionInSeconds);
    };

    spectrogramView.onSeek = [this](double positionInSeconds)
    {
        audioEngine.setPosition(positionInSeconds);
        waveformView.setPlayheadPosition(positionInSeconds);
        spectrogramView.setPlayheadPosition(positionInSeconds);
    };

    waveformView.onZoom = [this](double proportion, float deltaY) { handleZoom(proportion, deltaY); };
    spectrogramView.onZoom = [this](double proportion, float deltaY) { handleZoom(proportion, deltaY); };

    setWantsKeyboardFocus(true);

    setSize(800, 600);
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    constexpr int statusBarHeight = 28;
    statusBar.setBounds(bounds.removeFromBottom(statusBarHeight));

    constexpr int controlBarHeight = 74;
    controlBar.setBounds(bounds.removeFromTop(controlBarHeight));

    auto viewArea = bounds.reduced(8);

    const bool showRulers = (currentViewMode != ControlBar::ViewMode::details);

    constexpr int timeRulerHeight = 20;
    constexpr int verticalRulerWidth = 50;

    timeRuler.setVisible(showRulers);
    amplitudeRuler.setVisible(showRulers && currentViewMode == ControlBar::ViewMode::waveform);
    frequencyRuler.setVisible(showRulers && currentViewMode == ControlBar::ViewMode::spectrogram);

    if (showRulers)
    {
        auto timeRulerArea = viewArea.removeFromBottom(timeRulerHeight);
        auto verticalRulerArea = viewArea.removeFromLeft(verticalRulerWidth);
        timeRulerArea.removeFromLeft(verticalRulerWidth);

        timeRuler.setBounds(timeRulerArea);
        amplitudeRuler.setBounds(verticalRulerArea);
        frequencyRuler.setBounds(verticalRulerArea);
    }

    waveformView.setBounds(viewArea);
    spectrogramView.setBounds(viewArea);
    detailsView.setBounds(viewArea);
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::spaceKey)
    {
        if (audioEngine.isPlaying())
            audioEngine.pause();
        else
            audioEngine.play();

        return true;
    }

    return false;
}

void MainComponent::openFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Open a WAV file...",
        juce::File(),
        "*.wav");

    constexpr auto chooserFlags = juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();

        if (file == juce::File{})
            return;

        loadFile(file);
    });
}

void MainComponent::loadFile(const juce::File& file)
{
    if (audioEngine.loadFile(file))
    {
        waveformView.setFile(file);
        waveformView.setNumChannels(audioEngine.getNumChannels());
        spectrogramView.setFile(file);
        detailsView.setFile(file);

        totalDuration = audioEngine.getLengthInSeconds();
        visibleStart = 0.0;
        visibleEnd = totalDuration;
        updateVisibleRange();

        frequencyRuler.setSampleRate(audioEngine.getSampleRate());
        amplitudeRuler.setNumChannels(audioEngine.getNumChannels());
        statusBar.setMessage(file.getFileName() + " loaded successfully");
    }
    else
    {
        waveformView.clear();
        spectrogramView.clear();
        detailsView.clear();

        totalDuration = 0.0;
        visibleStart = 0.0;
        visibleEnd = 0.0;
        updateVisibleRange();

        statusBar.setMessage("Failed to load " + file.getFileName());
    }
}

void MainComponent::timerCallback()
{
    const auto position = audioEngine.getCurrentPosition();
    waveformView.setPlayheadPosition(position);
    spectrogramView.setPlayheadPosition(position);
}

void MainComponent::handleZoom(double mouseXProportion, float wheelDeltaY)
{
    if (totalDuration <= 0.0)
        return;

    constexpr double minSpanSeconds = 0.05;
    constexpr double zoomInFactor = 0.8;
    constexpr double zoomOutFactor = 1.25;

    const double currentSpan = (visibleEnd > visibleStart) ? (visibleEnd - visibleStart) : totalDuration;
    const double anchorTime = visibleStart + mouseXProportion * currentSpan;

    const double factor = (wheelDeltaY > 0.0f) ? zoomInFactor : zoomOutFactor;
    double newSpan = juce::jlimit(minSpanSeconds, totalDuration, currentSpan * factor);

    double newStart = anchorTime - mouseXProportion * newSpan;
    double newEnd = newStart + newSpan;

    if (newStart < 0.0)
    {
        newEnd -= newStart;
        newStart = 0.0;
    }

    if (newEnd > totalDuration)
    {
        newStart -= (newEnd - totalDuration);
        newEnd = totalDuration;
    }

    newStart = juce::jmax(0.0, newStart);

    visibleStart = newStart;
    visibleEnd = newEnd;

    updateVisibleRange();
}

void MainComponent::updateVisibleRange()
{
    waveformView.setVisibleRange(visibleStart, visibleEnd);
    spectrogramView.setVisibleRange(visibleStart, visibleEnd);
    timeRuler.setRange(visibleStart, visibleEnd);
}