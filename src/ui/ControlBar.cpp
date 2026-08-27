#include "ControlBar.h"

namespace
{
    juce::Path makePlayPath()
    {
        juce::Path p;
        p.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
        return p;
    }

    juce::Path makePausePath()
    {
        juce::Path p;
        p.addRectangle(0.0f, 0.0f, 0.35f, 1.0f);
        p.addRectangle(0.65f, 0.0f, 0.35f, 1.0f);
        return p;
    }

    juce::Path makeStopPath()
    {
        juce::Path p;
        p.addRectangle(0.0f, 0.0f, 1.0f, 1.0f);
        return p;
    }

    constexpr int fftSizeValues[]  = { 512, 1024, 2048, 4096, 8192 };
    constexpr int overlapValues[]  = { 0, 25, 50, 75 };
}

ControlBar::ControlBar()
{
    playButton.setShape(makePlayPath(), true, true, false);
    pauseButton.setShape(makePausePath(), true, true, false);
    stopButton.setShape(makeStopPath(), true, true, false);

    for (auto* b : { &playButton, &pauseButton, &stopButton })
    {
        b->setMouseClickGrabsKeyboardFocus(false);
        addAndMakeVisible(b);
    }

    playButton.onClick  = [this] { if (onPlayClicked)  onPlayClicked(); };
    pauseButton.onClick = [this] { if (onPauseClicked) onPauseClicked(); };
    stopButton.onClick  = [this] { if (onStopClicked)  onStopClicked(); };

    waveformViewButton.setClickingTogglesState(true);
    spectrogramViewButton.setClickingTogglesState(true);
    detailsViewButton.setClickingTogglesState(true);
    waveformViewButton.setRadioGroupId(1);
    spectrogramViewButton.setRadioGroupId(1);
    detailsViewButton.setRadioGroupId(1);
    waveformViewButton.setMouseClickGrabsKeyboardFocus(false);
    spectrogramViewButton.setMouseClickGrabsKeyboardFocus(false);
    detailsViewButton.setMouseClickGrabsKeyboardFocus(false);
    waveformViewButton.setToggleState(true, juce::dontSendNotification);

    waveformViewButton.onClick = [this]
    {
        currentViewMode = ViewMode::waveform;
        if (onViewModeChanged)
            onViewModeChanged(currentViewMode);
    };

    spectrogramViewButton.onClick = [this]
    {
        currentViewMode = ViewMode::spectrogram;
        if (onViewModeChanged)
            onViewModeChanged(currentViewMode);
    };

    detailsViewButton.onClick = [this]
    {
        currentViewMode = ViewMode::details;
        if (onViewModeChanged)
            onViewModeChanged(currentViewMode);
    };

    addAndMakeVisible(waveformViewButton);
    addAndMakeVisible(spectrogramViewButton);
    addAndMakeVisible(detailsViewButton);

    // --- STFT settings row ---

    windowTypeLabel.setJustificationType(juce::Justification::centredRight);
    fftSizeLabel.setJustificationType(juce::Justification::centredRight);
    overlapLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(windowTypeLabel);
    addAndMakeVisible(fftSizeLabel);
    addAndMakeVisible(overlapLabel);

    windowTypeCombo.addItem("Rectangular",     1);
    windowTypeCombo.addItem("Hann",            2);
    windowTypeCombo.addItem("Hamming",         3);
    windowTypeCombo.addItem("Blackman",        4);
    windowTypeCombo.addItem("Blackman-Harris", 5);
    windowTypeCombo.setSelectedId(2, juce::dontSendNotification); // Hann default

    fftSizeCombo.addItem("512",  1);
    fftSizeCombo.addItem("1024", 2);
    fftSizeCombo.addItem("2048", 3);
    fftSizeCombo.addItem("4096", 4);
    fftSizeCombo.addItem("8192", 5);
    fftSizeCombo.setSelectedId(3, juce::dontSendNotification); // 2048 default

    overlapCombo.addItem("0%",  1);
    overlapCombo.addItem("25%", 2);
    overlapCombo.addItem("50%", 3);
    overlapCombo.addItem("75%", 4);
    overlapCombo.setSelectedId(3, juce::dontSendNotification); // 50% default

    for (auto* combo : { &windowTypeCombo, &fftSizeCombo, &overlapCombo })
    {
        combo->setMouseClickGrabsKeyboardFocus(false);
        combo->onChange = [this] { notifySTFTSettingsChanged(); };
        addAndMakeVisible(combo);
    }
}

STFTSettings ControlBar::getCurrentSTFTSettings() const
{
    STFTSettings settings;

    switch (windowTypeCombo.getSelectedId())
    {
        case 1: settings.windowType = WindowType::rectangular;    break;
        case 2: settings.windowType = WindowType::hann;           break;
        case 3: settings.windowType = WindowType::hamming;        break;
        case 4: settings.windowType = WindowType::blackman;       break;
        case 5: settings.windowType = WindowType::blackmanHarris; break;
        default: settings.windowType = WindowType::hann;          break;
    }

    const int fftIdx = juce::jlimit(1, 5, fftSizeCombo.getSelectedId()) - 1;
    settings.fftSize = fftSizeValues[fftIdx];

    const int overlapIdx = juce::jlimit(1, 4, overlapCombo.getSelectedId()) - 1;
    settings.overlapPercent = overlapValues[overlapIdx];

    return settings;
}

void ControlBar::notifySTFTSettingsChanged()
{
    if (onSTFTSettingsChanged)
        onSTFTSettingsChanged(getCurrentSTFTSettings());
}

void ControlBar::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(0.05f));

    constexpr int topRowHeight = 28;

    g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
    g.drawLine(0.0f, static_cast<float>(getHeight()),
               static_cast<float>(getWidth()), static_cast<float>(getHeight()), 1.0f);

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(playButtonBoxBounds.toFloat(), 4.0f);
    g.fillRoundedRectangle(pauseButtonBoxBounds.toFloat(), 4.0f);
    g.fillRoundedRectangle(stopButtonBoxBounds.toFloat(), 4.0f);

    constexpr int dividerX = 150;
    g.drawLine(static_cast<float>(dividerX), 6.0f,
               static_cast<float>(dividerX), static_cast<float>(topRowHeight - 6), 1.0f);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    constexpr int topRowHeight = 28;
    auto topRow    = bounds.removeFromTop(topRowHeight);
    auto bottomRow = bounds;

    // --- Row 1: transport + view toggle ---

    auto transportArea = topRow.removeFromLeft(140);

    constexpr int iconButtonSize = 20;
    constexpr int spacing = 17;
    constexpr int boxPadding = 8;

    auto buttonRow = transportArea.withSizeKeepingCentre(
        (iconButtonSize * 3) + (spacing * 2), iconButtonSize);

    auto playArea = buttonRow.removeFromLeft(iconButtonSize);
    buttonRow.removeFromLeft(spacing);
    auto pauseArea = buttonRow.removeFromLeft(iconButtonSize);
    buttonRow.removeFromLeft(spacing);
    auto stopArea = buttonRow.removeFromLeft(iconButtonSize);

    playButton.setBounds(playArea);
    pauseButton.setBounds(pauseArea);
    stopButton.setBounds(stopArea);

    playButtonBoxBounds  = playArea.expanded(boxPadding);
    pauseButtonBoxBounds = pauseArea.expanded(boxPadding);
    stopButtonBoxBounds  = stopArea.expanded(boxPadding);

    topRow.removeFromLeft(spacing);

    constexpr int viewButtonWidth = 90;
    waveformViewButton.setBounds(topRow.removeFromLeft(viewButtonWidth));
    topRow.removeFromLeft(spacing);
    spectrogramViewButton.setBounds(topRow.removeFromLeft(viewButtonWidth));
    topRow.removeFromLeft(spacing);
    detailsViewButton.setBounds(topRow.removeFromLeft(viewButtonWidth));

    // --- Row 2: STFT settings ---

    constexpr int labelWidth  = 60;
    constexpr int comboWidth  = 90;
    constexpr int rowSpacing  = 10;
    constexpr int controlHeight = 22;

    auto placeLabelAndCombo = [&](juce::Label& label, juce::ComboBox& combo)
    {
        label.setBounds(bottomRow.removeFromLeft(labelWidth).withSizeKeepingCentre(labelWidth, controlHeight));
        combo.setBounds(bottomRow.removeFromLeft(comboWidth).withSizeKeepingCentre(comboWidth, controlHeight));
        bottomRow.removeFromLeft(rowSpacing);
    };

    placeLabelAndCombo(windowTypeLabel, windowTypeCombo);
    placeLabelAndCombo(fftSizeLabel, fftSizeCombo);
    placeLabelAndCombo(overlapLabel, overlapCombo);
}