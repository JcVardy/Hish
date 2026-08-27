#include "AudioEngine.h"

AudioEngine::AudioEngine(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
    auto error = deviceManager.initialiseWithDefaultDevices(0, 2);

    if (error.isNotEmpty())
        DBG("AudioDeviceManager failed to initialise: " + error);

    deviceManager.addAudioCallback(&sourcePlayer);
    sourcePlayer.setSource(&transportSource);
}

AudioEngine::~AudioEngine()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    sourcePlayer.setSource(nullptr);
    deviceManager.removeAudioCallback(&sourcePlayer);
}

bool AudioEngine::loadFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file);

    if (reader == nullptr)
        return false;

    auto newReaderSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

    transportSource.stop();
    transportSource.setSource(newReaderSource.get(),
                               0,
                               nullptr,
                               reader->sampleRate);

    lastSampleRate = reader->sampleRate;
    lastNumChannels = static_cast<int>(reader->numChannels);
    readerSource = std::move(newReaderSource);
    return true;
}

void AudioEngine::play()
{
    if (readerSource == nullptr)
        return;

    if (getLengthInSeconds() > 0.0 && getCurrentPosition() >= getLengthInSeconds() - 0.01)
        transportSource.setPosition(0.0);

    transportSource.start();
}

void AudioEngine::setPosition(double positionInSeconds)
{
    transportSource.setPosition(positionInSeconds);
}

void AudioEngine::pause()
{
    transportSource.stop();
}

void AudioEngine::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void AudioEngine::setGain(float newGain)
{
    transportSource.setGain(newGain);
}

bool AudioEngine::isPlaying() const
{
    return transportSource.isPlaying();
}

double AudioEngine::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

double AudioEngine::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double AudioEngine::getSampleRate() const
{
    return lastSampleRate;
}

int AudioEngine::getNumChannels() const
{
    return lastNumChannels;
}