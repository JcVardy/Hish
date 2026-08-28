#include "SpectrogramView.h"

namespace
{
    juce::Colour magnitudeToColour(float level)
    {
        level = juce::jlimit(0.0f, 1.0f, level);
        return juce::Colour::fromHSV(0.7f - 0.7f * level, 1.0f, level, 1.0f);
    }

    juce::dsp::WindowingFunction<float>::WindowingMethod toJuceWindowingMethod(WindowType type)
    {
        switch (type)
        {
            case WindowType::rectangular:    return juce::dsp::WindowingFunction<float>::rectangular;
            case WindowType::hann:           return juce::dsp::WindowingFunction<float>::hann;
            case WindowType::hamming:        return juce::dsp::WindowingFunction<float>::hamming;
            case WindowType::blackman:       return juce::dsp::WindowingFunction<float>::blackman;
            case WindowType::blackmanHarris: return juce::dsp::WindowingFunction<float>::blackmanHarris;
        }
        return juce::dsp::WindowingFunction<float>::hann;
    }

    constexpr int dragThreshold = 4;
}

int SpectrogramView::orderForSize(int size)
{
    return static_cast<int>(std::round(std::log2(static_cast<double>(size))));
}

SpectrogramView::SpectrogramView(juce::AudioFormatManager& formatManagerToUse)
    : juce::Thread("SpectrogramAnalysis"), formatManager(formatManagerToUse)
{
}

SpectrogramView::~SpectrogramView()
{
    stopThread(4000);
}

void SpectrogramView::setFile(const juce::File& file)
{
    stopThread(4000);

    currentFile = file;
    hasImage = false;
    playheadPosition = 0.0;
    visibleStart = 0.0;
    visibleEnd = 0.0;
    visibleFreqMin = 0.0;
    visibleFreqMax = 0.0;

    startThread();
    repaint();
}

void SpectrogramView::clear()
{
    stopThread(4000);
    currentFile = juce::File();
    visibleStart = 0.0;
    visibleEnd = 0.0;
    visibleFreqMin = 0.0;
    visibleFreqMax = 0.0;

    {
        const juce::ScopedLock sl(imageLock);
        spectrogramImage = juce::Image();
        hasImage = false;
    }

    repaint();
}

void SpectrogramView::setPlayheadPosition(double positionInSeconds)
{
    playheadPosition = positionInSeconds;
    repaint();
}

void SpectrogramView::setVisibleRange(double startSeconds, double endSeconds)
{
    visibleStart = startSeconds;
    visibleEnd = endSeconds;
    repaint();
}

void SpectrogramView::setNyquist(double newNyquist)
{
    nyquist = juce::jmax(1.0, newNyquist);
    visibleFreqMin = 0.0;
    visibleFreqMax = nyquist;
    repaint();
}

void SpectrogramView::setVisibleFrequencyRange(double minHz, double maxHz)
{
    visibleFreqMin = minHz;
    visibleFreqMax = maxHz;
    repaint();
}

void SpectrogramView::setSTFTSettings(const STFTSettings& settings)
{
    fftSize         = settings.fftSize;
    fftOrder        = orderForSize(fftSize);
    hopSize         = juce::jmax(1, (fftSize * (100 - settings.overlapPercent)) / 100);
    windowingMethod = toJuceWindowingMethod(settings.windowType);

    if (currentFile != juce::File())
    {
        stopThread(4000);
        hasImage = false;
        startThread();
        repaint();
    }
}

void SpectrogramView::run()
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(currentFile));

    if (reader == nullptr)
        return;

    const auto numSamples = static_cast<int>(reader->lengthInSamples);

    if (numSamples <= 0)
        return;

    fileLengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

    juce::AudioBuffer<float> buffer(1, numSamples);
    reader->read(&buffer, 0, numSamples, 0, true, false);

    const int currentFftSize  = fftSize;
    const int currentFftOrder = fftOrder;
    const int currentHopSize  = hopSize;

    const int numFrames  = juce::jmax(1, (numSamples - currentFftSize) / currentHopSize);
    const int imageWidth  = juce::jlimit(1, 2000, numFrames);
    const int imageHeight = currentFftSize / 2;

    juce::Image image(juce::Image::RGB, imageWidth, imageHeight, true);

    juce::dsp::FFT fft(currentFftOrder);
    juce::dsp::WindowingFunction<float> window(static_cast<size_t>(currentFftSize), windowingMethod, false);
    juce::HeapBlock<float> fftData(static_cast<size_t>(currentFftSize) * 2);

    for (int x = 0; x < imageWidth; ++x)
    {
        if (threadShouldExit())
            return;

        const int frameIndex  = (imageWidth > 1) ? (x * (numFrames - 1)) / (imageWidth - 1) : 0;
        const int sampleStart = frameIndex * currentHopSize;
        const int samplesAvailable = juce::jmin(currentFftSize, numSamples - sampleStart);

        fftData.clear(static_cast<size_t>(currentFftSize) * 2);

        if (samplesAvailable > 0)
        {
            auto* channelData = buffer.getReadPointer(0, sampleStart);
            std::copy(channelData, channelData + samplesAvailable, fftData.getData());
        }

        window.multiplyWithWindowingTable(fftData.getData(), static_cast<size_t>(currentFftSize));

        fft.performFrequencyOnlyForwardTransform(fftData.getData());

        for (int y = 0; y < imageHeight; ++y)
        {
            const float magnitude = fftData[y];
            const float db    = juce::Decibels::gainToDecibels(magnitude, -100.0f);
            const float level = juce::jmap(db, -100.0f, 0.0f, 0.0f, 1.0f);

            image.setPixelAt(x, imageHeight - 1 - y, magnitudeToColour(level));
        }
    }

    if (threadShouldExit())
        return;

    {
        const juce::ScopedLock sl(imageLock);
        spectrogramImage = image;
        hasImage = true;
    }

    juce::MessageManager::callAsync([this] { repaint(); });
}

void SpectrogramView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds);
    juce::Image imageToDraw;
    bool imageReady = false;
    {
        const juce::ScopedLock sl(imageLock);
        imageReady = hasImage;
        if (imageReady)
            imageToDraw = spectrogramImage;
    }
    if (imageReady && fileLengthInSeconds > 0.0)
    {
        const double rangeStart = (visibleEnd > visibleStart) ? visibleStart : 0.0;
        const double rangeEnd   = (visibleEnd > visibleStart) ? visibleEnd   : fileLengthInSeconds;
        const double freqMin = (visibleFreqMax > visibleFreqMin) ? visibleFreqMin : 0.0;
        const double freqMax = (visibleFreqMax > visibleFreqMin) ? visibleFreqMax : nyquist;
        const int imgWidth  = imageToDraw.getWidth();
        const int imgHeight = imageToDraw.getHeight();
        const int srcX     = juce::jlimit(0, imgWidth, static_cast<int>((rangeStart / fileLengthInSeconds) * imgWidth));
        const int srcRight = juce::jlimit(0, imgWidth, static_cast<int>((rangeEnd / fileLengthInSeconds) * imgWidth));
        const int srcWidth = juce::jmax(1, srcRight - srcX);
        const int srcY      = juce::jlimit(0, imgHeight, static_cast<int>(imgHeight * (1.0 - freqMax / nyquist)));
        const int srcBottom = juce::jlimit(0, imgHeight, static_cast<int>(imgHeight * (1.0 - freqMin / nyquist)));
        const int srcHeight = juce::jmax(1, srcBottom - srcY);

        constexpr int outerMargin = 14;
        auto destArea = bounds.reduced(2);
        destArea.removeFromTop(outerMargin);
        destArea.removeFromBottom(outerMargin);

        g.drawImage(imageToDraw,
                    destArea.getX(), destArea.getY(), destArea.getWidth(), destArea.getHeight(),
                    srcX, srcY, srcWidth, srcHeight);
        if (playheadPosition >= rangeStart && playheadPosition <= rangeEnd)
        {
            const auto proportion = (playheadPosition - rangeStart) / juce::jmax(0.0001, rangeEnd - rangeStart);
            const auto x = bounds.getX() + static_cast<int>(proportion * bounds.getWidth());
            g.setColour(juce::Colours::red);
            g.drawLine(static_cast<float>(x), static_cast<float>(bounds.getY()),
                       static_cast<float>(x), static_cast<float>(bounds.getBottom()), 2.0f);
        }
    }
    else if (isThreadRunning())
    {
        g.setColour(juce::Colours::grey);
        g.drawText("Analyzing...", bounds, juce::Justification::centred);
    }
    else
    {
        g.setColour(juce::Colours::grey);
        g.drawText("No file loaded", bounds, juce::Justification::centred);
    }
}

void SpectrogramView::resized()
{
}

void SpectrogramView::mouseDown(const juce::MouseEvent& event)
{
    lastDragPosition = event.getPosition();
    isDraggingPastThreshold = false;
}

void SpectrogramView::mouseDrag(const juce::MouseEvent& event)
{
    if (fileLengthInSeconds <= 0.0)
        return;

    if (!isDraggingPastThreshold)
    {
        if (event.getDistanceFromDragStart() < dragThreshold)
            return;

        isDraggingPastThreshold = true;
        lastDragPosition = event.getPosition();
    }

    const auto pos = event.getPosition();
    const int dx = pos.x - lastDragPosition.x;
    const int dy = pos.y - lastDragPosition.y;
    lastDragPosition = pos;

    if (dx != 0 && onPan && getWidth() > 0)
    {
        const double currentSpan = (visibleEnd > visibleStart) ? (visibleEnd - visibleStart) : 0.0;
        const double deltaTime = -(static_cast<double>(dx) / getWidth()) * currentSpan;
        onPan(deltaTime);
    }

    if (dy != 0 && onVerticalPan && getHeight() > 0)
    {
        const double currentFreqSpan = (visibleFreqMax > visibleFreqMin) ? (visibleFreqMax - visibleFreqMin) : nyquist;
        const double deltaHz = (static_cast<double>(dy) / getHeight()) * currentFreqSpan;
        onVerticalPan(deltaHz);
    }
}

void SpectrogramView::mouseUp(const juce::MouseEvent& event)
{
    if (isDraggingPastThreshold)
    {
        isDraggingPastThreshold = false;
        return;
    }

    if (!hasImage || fileLengthInSeconds <= 0.0)
        return;

    auto bounds = getLocalBounds();

    const double rangeStart = (visibleEnd > visibleStart) ? visibleStart : 0.0;
    const double rangeEnd   = (visibleEnd > visibleStart) ? visibleEnd   : fileLengthInSeconds;

    const auto proportion = juce::jlimit(0.0, 1.0,
        (event.x - bounds.getX()) / static_cast<double>(bounds.getWidth()));

    const auto newPosition = rangeStart + proportion * (rangeEnd - rangeStart);

    if (onSeek)
        onSeek(newPosition);
}

void SpectrogramView::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (fileLengthInSeconds <= 0.0)
        return;

    auto bounds = getLocalBounds();

    if (event.mods.isCtrlDown())
    {
        const auto yProportion = juce::jlimit(0.0, 1.0,
            (event.y - bounds.getY()) / static_cast<double>(bounds.getHeight()));

        if (onVerticalZoom)
            onVerticalZoom(yProportion, wheel.deltaY);
        return;
    }

    const auto xProportion = juce::jlimit(0.0, 1.0,
        (event.x - bounds.getX()) / static_cast<double>(bounds.getWidth()));

    if (onZoom)
        onZoom(xProportion, wheel.deltaY);
}