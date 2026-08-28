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

    addAndMakeVisible(horizontalNavBar);
    addAndMakeVisible(verticalNavBar);

    horizontalNavBar.getScrollBar().addListener(this);
    verticalNavBar.getScrollBar().addListener(this);

    horizontalNavBar.onZoomInClicked  = [this] { handleZoom(0.5, 1.0f); };
    horizontalNavBar.onZoomOutClicked = [this] { handleZoom(0.5, -1.0f); };
    verticalNavBar.onZoomInClicked    = [this] { handleVerticalZoomButton(true); };
    verticalNavBar.onZoomOutClicked   = [this] { handleVerticalZoomButton(false); };

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
        updateScrollBars();
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

    waveformView.onPan = [this](double deltaTime) { handlePan(deltaTime); };
    spectrogramView.onPan = [this](double deltaTime) { handlePan(deltaTime); };

    waveformView.onVerticalZoom = [this](float deltaY) { handleWaveformVerticalZoom(deltaY); };
    spectrogramView.onVerticalZoom = [this](double yProportion, float deltaY)
    {
        handleSpectrogramVerticalZoom(yProportion, deltaY);
    };

    waveformView.onVerticalPan = [this](double deltaAmplitude) { handleWaveformVerticalPan(deltaAmplitude); };
    spectrogramView.onVerticalPan = [this](double deltaHz) { handleSpectrogramVerticalPan(deltaHz); };

    setWantsKeyboardFocus(true);

    setSize(800, 600);
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    horizontalNavBar.getScrollBar().removeListener(this);
    verticalNavBar.getScrollBar().removeListener(this);
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

    constexpr int navBarThickness = 20;

    verticalNavBar.setBounds(bounds.removeFromRight(navBarThickness));
    horizontalNavBar.setBounds(bounds.removeFromBottom(navBarThickness));

    auto viewArea = bounds.reduced(8);

    const bool showRulers = (currentViewMode != ControlBar::ViewMode::details);

    constexpr int timeRulerHeight = 20;
    constexpr int verticalRulerWidth = 50;

    timeRuler.setVisible(showRulers);
    amplitudeRuler.setVisible(showRulers && currentViewMode == ControlBar::ViewMode::waveform);
    frequencyRuler.setVisible(showRulers && currentViewMode == ControlBar::ViewMode::spectrogram);
    horizontalNavBar.setVisible(showRulers);
    verticalNavBar.setVisible(showRulers);

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

    if (key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier, 0))
    {
        performUndo();
        return true;
    }

    if (key == juce::KeyPress('z', juce::ModifierKeys::ctrlModifier | juce::ModifierKeys::shiftModifier, 0))
    {
        performRedo();
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

void MainComponent::exportFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Export WAV file...",
        juce::File(),
        "*.wav");

    constexpr auto chooserFlags = juce::FileBrowserComponent::saveMode
                                 | juce::FileBrowserComponent::canSelectFiles
                                 | juce::FileBrowserComponent::warnAboutOverwriting;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        const auto file = fc.getResult();

        if (file == juce::File{})
            return;

        if (audioEngine.exportToFile(file))
            statusBar.setMessage("Exported to " + file.getFileName());
        else
            statusBar.setMessage("Export failed");
    });
}

void MainComponent::loadFile(const juce::File& file, bool addToUndoHistory)
{
    const auto previousFile = audioEngine.getCurrentFile();

    if (audioEngine.loadFile(file))
    {
        if (addToUndoHistory && previousFile != juce::File())
        {
            undoHistory.push_back(previousFile);
            redoHistory.clear();
        }

        waveformView.setFile(file);
        waveformView.setNumChannels(audioEngine.getNumChannels());
        spectrogramView.setFile(file);
        detailsView.setFile(file);
        totalDuration = audioEngine.getLengthInSeconds();
        visibleStart = 0.0;
        visibleEnd = totalDuration;
        updateVisibleRange();

        waveformVerticalZoom = 1.0f;
        waveformPanCenter = 0.0f;
        waveformView.setVerticalZoom(waveformVerticalZoom);
        waveformView.setPanCenter(waveformPanCenter);
        amplitudeRuler.setRange(-1.0f, 1.0f);

        spectrogramNyquist = audioEngine.getSampleRate() / 2.0;
        spectrogramFreqMin = 0.0;
        spectrogramFreqMax = spectrogramNyquist;
        spectrogramView.setNyquist(spectrogramNyquist);
        frequencyRuler.setRange(spectrogramFreqMin, spectrogramFreqMax);

        amplitudeRuler.setNumChannels(audioEngine.getNumChannels());
        statusBar.setMessage(file.getFileName() + " loaded successfully");

        updateScrollBars();
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
        updateScrollBars();
    }
}

void MainComponent::performUndo()
{
    if (undoHistory.empty())
    {
        statusBar.setMessage("Nothing to undo");
        return;
    }

    const auto currentFile = audioEngine.getCurrentFile();
    const auto previousFile = undoHistory.back();
    undoHistory.pop_back();

    if (currentFile != juce::File())
        redoHistory.push_back(currentFile);

    loadFile(previousFile, false);
    statusBar.setMessage("Undo: reverted to " + previousFile.getFileName());
}

void MainComponent::performRedo()
{
    if (redoHistory.empty())
    {
        statusBar.setMessage("Nothing to redo");
        return;
    }

    const auto currentFile = audioEngine.getCurrentFile();
    const auto nextFile = redoHistory.back();
    redoHistory.pop_back();

    if (currentFile != juce::File())
        undoHistory.push_back(currentFile);

    loadFile(nextFile, false);
    statusBar.setMessage("Redo: advanced to " + nextFile.getFileName());
}

void MainComponent::timerCallback()
{
    const auto position = audioEngine.getCurrentPosition();
    waveformView.setPlayheadPosition(position);
    spectrogramView.setPlayheadPosition(position);
}

void MainComponent::scrollBarMoved(juce::ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == &horizontalNavBar.getScrollBar())
    {
        const double span = (visibleEnd > visibleStart) ? (visibleEnd - visibleStart) : totalDuration;

        visibleStart = newRangeStart;
        visibleEnd = visibleStart + span;
        updateVisibleRange();
    }
    else if (scrollBarThatHasMoved == &verticalNavBar.getScrollBar())
    {
        if (currentViewMode == ControlBar::ViewMode::waveform)
        {
            const float visibleHalf = 1.0f / waveformVerticalZoom;

            const float ampMax = 1.0f - static_cast<float>(newRangeStart);
            const float maxPan = 1.0f - visibleHalf;

            waveformPanCenter = juce::jlimit(-maxPan, maxPan, ampMax - visibleHalf);

            waveformView.setPanCenter(waveformPanCenter);
            amplitudeRuler.setRange(waveformPanCenter - visibleHalf, waveformPanCenter + visibleHalf);
        }
        else if (currentViewMode == ControlBar::ViewMode::spectrogram)
        {
            const double span = (spectrogramFreqMax > spectrogramFreqMin)
                                     ? (spectrogramFreqMax - spectrogramFreqMin)
                                     : spectrogramNyquist;

            spectrogramFreqMax = juce::jlimit(0.0, spectrogramNyquist, spectrogramNyquist - newRangeStart);
            spectrogramFreqMin = juce::jmax(0.0, spectrogramFreqMax - span);

            spectrogramView.setVisibleFrequencyRange(spectrogramFreqMin, spectrogramFreqMax);
            frequencyRuler.setRange(spectrogramFreqMin, spectrogramFreqMax);
        }
    }
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

void MainComponent::handlePan(double deltaTimeSeconds)
{
    if (totalDuration <= 0.0)
        return;

    const double span = (visibleEnd > visibleStart) ? (visibleEnd - visibleStart) : totalDuration;

    double newStart = visibleStart + deltaTimeSeconds;
    double newEnd = newStart + span;

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

    visibleStart = juce::jmax(0.0, newStart);
    visibleEnd = newEnd;

    updateVisibleRange();
}

void MainComponent::handleWaveformVerticalZoom(float wheelDeltaY)
{
    const float factor = (wheelDeltaY > 0.0f) ? 1.25f : 0.8f;

    waveformVerticalZoom = juce::jlimit(1.0f, 20.0f, waveformVerticalZoom * factor);

    const float maxPan = 1.0f - 1.0f / waveformVerticalZoom;
    waveformPanCenter = juce::jlimit(-maxPan, maxPan, waveformPanCenter);

    waveformView.setVerticalZoom(waveformVerticalZoom);
    waveformView.setPanCenter(waveformPanCenter);

    const float visibleHalf = 1.0f / waveformVerticalZoom;
    amplitudeRuler.setRange(waveformPanCenter - visibleHalf, waveformPanCenter + visibleHalf);

    updateScrollBars();
}

void MainComponent::handleWaveformVerticalPan(double deltaAmplitude)
{
    const float maxPan = 1.0f - 1.0f / waveformVerticalZoom;

    if (maxPan <= 0.0f)
        return;

    waveformPanCenter = juce::jlimit(-maxPan, maxPan,
        waveformPanCenter + static_cast<float>(deltaAmplitude));

    waveformView.setPanCenter(waveformPanCenter);

    const float visibleHalf = 1.0f / waveformVerticalZoom;
    amplitudeRuler.setRange(waveformPanCenter - visibleHalf, waveformPanCenter + visibleHalf);

    updateScrollBars();
}

void MainComponent::handleSpectrogramVerticalZoom(double mouseYProportion, float wheelDeltaY)
{
    if (spectrogramNyquist <= 0.0)
        return;

    constexpr double minSpanHz = 20.0;
    constexpr double zoomInFactor = 0.8;
    constexpr double zoomOutFactor = 1.25;

    const double currentSpan = (spectrogramFreqMax > spectrogramFreqMin)
                                   ? (spectrogramFreqMax - spectrogramFreqMin)
                                   : spectrogramNyquist;

    const double p = 1.0 - mouseYProportion;
    const double anchorFreq = spectrogramFreqMin + p * currentSpan;

    const double factor = (wheelDeltaY > 0.0f) ? zoomInFactor : zoomOutFactor;
    double newSpan = juce::jlimit(minSpanHz, spectrogramNyquist, currentSpan * factor);

    double newMin = anchorFreq - p * newSpan;
    double newMax = newMin + newSpan;

    if (newMin < 0.0)
    {
        newMax -= newMin;
        newMin = 0.0;
    }

    if (newMax > spectrogramNyquist)
    {
        newMin -= (newMax - spectrogramNyquist);
        newMax = spectrogramNyquist;
    }

    spectrogramFreqMin = juce::jmax(0.0, newMin);
    spectrogramFreqMax = newMax;

    spectrogramView.setVisibleFrequencyRange(spectrogramFreqMin, spectrogramFreqMax);
    frequencyRuler.setRange(spectrogramFreqMin, spectrogramFreqMax);

    updateScrollBars();
}

void MainComponent::handleSpectrogramVerticalPan(double deltaHz)
{
    if (spectrogramNyquist <= 0.0)
        return;

    const double span = (spectrogramFreqMax > spectrogramFreqMin)
                             ? (spectrogramFreqMax - spectrogramFreqMin)
                             : spectrogramNyquist;

    double newMin = spectrogramFreqMin + deltaHz;
    double newMax = newMin + span;

    if (newMin < 0.0)
    {
        newMax -= newMin;
        newMin = 0.0;
    }

    if (newMax > spectrogramNyquist)
    {
        newMin -= (newMax - spectrogramNyquist);
        newMax = spectrogramNyquist;
    }

    spectrogramFreqMin = juce::jmax(0.0, newMin);
    spectrogramFreqMax = newMax;

    spectrogramView.setVisibleFrequencyRange(spectrogramFreqMin, spectrogramFreqMax);
    frequencyRuler.setRange(spectrogramFreqMin, spectrogramFreqMax);

    updateScrollBars();
}

void MainComponent::handleVerticalZoomButton(bool zoomIn)
{
    const float deltaY = zoomIn ? 1.0f : -1.0f;

    if (currentViewMode == ControlBar::ViewMode::waveform)
        handleWaveformVerticalZoom(deltaY);
    else if (currentViewMode == ControlBar::ViewMode::spectrogram)
        handleSpectrogramVerticalZoom(0.5, deltaY);
}

void MainComponent::updateVisibleRange()
{
    waveformView.setVisibleRange(visibleStart, visibleEnd);
    spectrogramView.setVisibleRange(visibleStart, visibleEnd);
    timeRuler.setRange(visibleStart, visibleEnd);

    updateScrollBars();
}

void MainComponent::updateScrollBars()
{
    auto& hScrollBar = horizontalNavBar.getScrollBar();
    auto& vScrollBar = verticalNavBar.getScrollBar();

    if (totalDuration > 0.0)
    {
        hScrollBar.setRangeLimits(0.0, totalDuration, juce::dontSendNotification);
        const double span = (visibleEnd > visibleStart) ? (visibleEnd - visibleStart) : totalDuration;
        hScrollBar.setCurrentRange(visibleStart, span, juce::dontSendNotification);
    }
    else
    {
        hScrollBar.setRangeLimits(0.0, 1.0, juce::dontSendNotification);
        hScrollBar.setCurrentRange(0.0, 1.0, juce::dontSendNotification);
    }

    if (currentViewMode == ControlBar::ViewMode::waveform)
    {
        const float visibleHalf = 1.0f / waveformVerticalZoom;
        const float ampMax = waveformPanCenter + visibleHalf;

        vScrollBar.setRangeLimits(0.0, 2.0, juce::dontSendNotification);
        vScrollBar.setCurrentRange(1.0 - ampMax, 2.0 * visibleHalf, juce::dontSendNotification);
    }
    else if (currentViewMode == ControlBar::ViewMode::spectrogram)
    {
        vScrollBar.setRangeLimits(0.0, spectrogramNyquist, juce::dontSendNotification);
        const double span = (spectrogramFreqMax > spectrogramFreqMin)
                                 ? (spectrogramFreqMax - spectrogramFreqMin)
                                 : spectrogramNyquist;
        vScrollBar.setCurrentRange(spectrogramNyquist - spectrogramFreqMax, span,
                                    juce::dontSendNotification);
    }
    else
    {
        vScrollBar.setRangeLimits(0.0, 1.0, juce::dontSendNotification);
        vScrollBar.setCurrentRange(0.0, 1.0, juce::dontSendNotification);
    }
}

void MainComponent::handleProcessSelected(ProcessType type)
{
    const auto sourceFile = audioEngine.getCurrentFile();

    if (sourceFile == juce::File())
    {
        statusBar.setMessage("Load a file before applying a process");
        return;
    }

    if (type == ProcessType::normalize)
        applyNormalizeProcess(sourceFile);
    else if (type == ProcessType::gain)
        promptForGain(sourceFile);
    else if (type == ProcessType::firFilter)
        promptForFIRFilter(sourceFile);
    else if (type == ProcessType::convolution)
        promptForConvolution(sourceFile);
    else if (type == ProcessType::iirFilter)
        promptForIIRFilter(sourceFile);
    else
        statusBar.setMessage("Process not yet implemented");
}

void MainComponent::applyNormalizeProcess(const juce::File& sourceFile)
{
    auto processedFile = processingEngine.applyNormalize(sourceFile);

    if (processedFile == juce::File())
    {
        statusBar.setMessage("Normalize failed");
        return;
    }

    loadFile(processedFile);
    statusBar.setMessage("Normalized " + sourceFile.getFileName());
}

void MainComponent::promptForGain(const juce::File& sourceFile)
{
    auto* alertWindow = new juce::AlertWindow("Apply Gain",
                                               "Enter gain in decibels (dB):",
                                               juce::MessageBoxIconType::NoIcon);

    alertWindow->addTextEditor("gainDb", "0.0", "Gain (dB)");
    alertWindow->addButton("Apply", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alertWindow, sourceFile](int result)
        {
            std::unique_ptr<juce::AlertWindow> ownedWindow(alertWindow);

            if (result != 1)
                return;

            const float gainDb = ownedWindow->getTextEditorContents("gainDb").getFloatValue();

            auto processedFile = processingEngine.applyGain(sourceFile, gainDb);

            if (processedFile == juce::File())
            {
                statusBar.setMessage("Apply Gain failed");
                return;
            }

            loadFile(processedFile);
            statusBar.setMessage("Applied " + juce::String(gainDb, 1) + " dB gain");
        }), true);
}



void MainComponent::promptForConvolution(const juce::File& sourceFile)
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select an impulse response WAV file...",
        juce::File(),
        "*.wav");

    constexpr auto chooserFlags = juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this, sourceFile](const juce::FileChooser& fc)
    {
        const auto irFile = fc.getResult();

        if (irFile == juce::File{})
            return;

        auto* alertWindow = new juce::AlertWindow("Convolution",
                                                   "Wet/dry mix (0 = dry only, 100 = wet only):",
                                                   juce::MessageBoxIconType::NoIcon);

        alertWindow->addTextEditor("mix", "50.0", "Mix (%)");
        alertWindow->addButton("Apply", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, alertWindow, sourceFile, irFile](int result)
            {
                std::unique_ptr<juce::AlertWindow> ownedWindow(alertWindow);

                if (result != 1)
                    return;

                const float mixPercent = juce::jlimit(0.0f, 100.0f,
                    ownedWindow->getTextEditorContents("mix").getFloatValue());
                const float wetDryMix = mixPercent / 100.0f;

                statusBar.setMessage("Applying convolution...");

                auto processedFile = processingEngine.applyConvolution(sourceFile, irFile, wetDryMix);

                if (processedFile == juce::File())
                {
                    statusBar.setMessage("Convolution failed");
                    return;
                }

                loadFile(processedFile);
                statusBar.setMessage("Applied convolution with " + irFile.getFileName()
                    + " (" + juce::String(mixPercent, 0) + "% wet)");
            }), true);
    });
}

void MainComponent::promptForFIRFilter(const juce::File& sourceFile)
{
    auto* alertWindow = new juce::AlertWindow("FIR Filter",
                                               "Choose filter type and cutoff frequency:",
                                               juce::MessageBoxIconType::NoIcon);

    juce::StringArray filterTypes { "Low-pass", "High-pass" };
    alertWindow->addComboBox("filterType", filterTypes, "Filter type");
    alertWindow->addTextEditor("cutoffHz", "1000.0", "Cutoff frequency (Hz)");

    auto responseView = std::make_shared<FilterResponseView>();
    alertWindow->addCustomComponent(responseView.get());

    const double previewSampleRate = audioEngine.getSampleRate();

    auto updateResponse = [alertWindow, responseView, previewSampleRate]
    {
        const auto typeIndex = alertWindow->getComboBoxComponent("filterType")->getSelectedItemIndex();
        const auto type = (typeIndex == 0) ? FIRFilterProcessor::FilterType::lowPass
                                            : FIRFilterProcessor::FilterType::highPass;
        const float cutoffHz = alertWindow->getTextEditorContents("cutoffHz").getFloatValue();

        auto kernel = FIRFilterProcessor::designKernel(previewSampleRate, cutoffHz, type);
        auto response = FilterResponseCalculator::computeFIRResponse(kernel, previewSampleRate);

        responseView->setResponse(response.magnitudesDb, response.phasesDegrees, kernel);
    };

    alertWindow->getComboBoxComponent("filterType")->onChange = updateResponse;
    alertWindow->getTextEditor("cutoffHz")->onTextChange = updateResponse;

    updateResponse();

    alertWindow->addButton("Apply", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alertWindow, sourceFile, responseView](int result)
        {
            std::unique_ptr<juce::AlertWindow> ownedWindow(alertWindow);
            juce::ignoreUnused(responseView);

            if (result != 1)
                return;

            const auto typeIndex = ownedWindow->getComboBoxComponent("filterType")->getSelectedItemIndex();
            const auto type = (typeIndex == 0) ? FIRFilterProcessor::FilterType::lowPass
                                                : FIRFilterProcessor::FilterType::highPass;

            const float cutoffHz = ownedWindow->getTextEditorContents("cutoffHz").getFloatValue();

            statusBar.setMessage("Applying FIR filter...");

            auto processedFile = processingEngine.applyFIRFilter(sourceFile, cutoffHz, type);

            if (processedFile == juce::File())
            {
                statusBar.setMessage("FIR filter failed");
                return;
            }

            loadFile(processedFile);
            statusBar.setMessage("Applied FIR "
                + juce::String(typeIndex == 0 ? "low-pass" : "high-pass")
                + " filter at " + juce::String(cutoffHz, 0) + " Hz");
        }), true);
}

void MainComponent::promptForIIRFilter(const juce::File& sourceFile)
{
    auto* alertWindow = new juce::AlertWindow("IIR Filter",
                                               "Choose filter type, frequency, and Q:",
                                               juce::MessageBoxIconType::NoIcon);

    juce::StringArray filterTypes { "Low-pass", "High-pass", "Band-pass", "Notch" };
    alertWindow->addComboBox("filterType", filterTypes, "Filter type");
    alertWindow->addTextEditor("frequencyHz", "1000.0", "Frequency (Hz)");
    alertWindow->addTextEditor("q", "0.707", "Q");

    auto responseView = std::make_shared<FilterResponseView>();
    alertWindow->addCustomComponent(responseView.get());

    const double previewSampleRate = audioEngine.getSampleRate();

    auto updateResponse = [alertWindow, responseView, previewSampleRate]
    {
        const auto typeIndex = alertWindow->getComboBoxComponent("filterType")->getSelectedItemIndex();

        IIRFilterProcessor::FilterType type;
        switch (typeIndex)
        {
            case 0: type = IIRFilterProcessor::FilterType::lowPass;  break;
            case 1: type = IIRFilterProcessor::FilterType::highPass; break;
            case 2: type = IIRFilterProcessor::FilterType::bandPass; break;
            default: type = IIRFilterProcessor::FilterType::notch;   break;
        }

        const float frequencyHz = alertWindow->getTextEditorContents("frequencyHz").getFloatValue();
        const float q = alertWindow->getTextEditorContents("q").getFloatValue();

        auto coeffs = IIRFilterProcessor::makeCoefficients(previewSampleRate, frequencyHz, q, type);
        auto response = FilterResponseCalculator::computeIIRResponse(coeffs, previewSampleRate);
        auto impulse = FilterResponseCalculator::computeIIRImpulseResponse(coeffs);

        responseView->setResponse(response.magnitudesDb, response.phasesDegrees, impulse);
    };

    alertWindow->getComboBoxComponent("filterType")->onChange = updateResponse;
    alertWindow->getTextEditor("frequencyHz")->onTextChange = updateResponse;
    alertWindow->getTextEditor("q")->onTextChange = updateResponse;

    updateResponse();

    alertWindow->addButton("Apply", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alertWindow, sourceFile, responseView](int result)
        {
            std::unique_ptr<juce::AlertWindow> ownedWindow(alertWindow);
            juce::ignoreUnused(responseView);

            if (result != 1)
                return;

            const auto typeIndex = ownedWindow->getComboBoxComponent("filterType")->getSelectedItemIndex();

            IIRFilterProcessor::FilterType type;
            juce::String typeName;

            switch (typeIndex)
            {
                case 0: type = IIRFilterProcessor::FilterType::lowPass;  typeName = "low-pass";  break;
                case 1: type = IIRFilterProcessor::FilterType::highPass; typeName = "high-pass"; break;
                case 2: type = IIRFilterProcessor::FilterType::bandPass; typeName = "band-pass"; break;
                default: type = IIRFilterProcessor::FilterType::notch;  typeName = "notch";      break;
            }

            const float frequencyHz = ownedWindow->getTextEditorContents("frequencyHz").getFloatValue();
            const float q = ownedWindow->getTextEditorContents("q").getFloatValue();

            statusBar.setMessage("Applying IIR filter...");

            auto processedFile = processingEngine.applyIIRFilter(sourceFile, frequencyHz, q, type);

            if (processedFile == juce::File())
            {
                statusBar.setMessage("IIR filter failed");
                return;
            }

            loadFile(processedFile);
            statusBar.setMessage("Applied IIR " + typeName + " filter at "
                + juce::String(frequencyHz, 0) + " Hz, Q=" + juce::String(q, 2));
        }), true);
}