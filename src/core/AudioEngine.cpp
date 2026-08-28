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
    transportSource.setSource(nullptr);

    auto currentSetup = deviceManager.getAudioDeviceSetup();

    if (currentSetup.sampleRate != reader->sampleRate)
    {
        currentSetup.sampleRate = reader->sampleRate;
        const auto error = deviceManager.setAudioDeviceSetup(currentSetup, true);

        if (error.isNotEmpty())
            DBG("Could not set device sample rate to match file: " + error);
    }

    transportSource.setSource(newReaderSource.get(), 0, nullptr, reader->sampleRate);

    currentAudioFile = file;
    lastSampleRate = reader->sampleRate;
    lastNumChannels = static_cast<int>(reader->numChannels);
    lastBitsPerSample = static_cast<int>(reader->bitsPerSample);

    readerSource = std::move(newReaderSource);
    return true;
}

bool AudioEngine::exportToFile(const juce::File& destFile) const
{
    if (currentAudioFile == juce::File())
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(currentAudioFile));

    if (reader == nullptr)
        return false;

    const auto numSamples = static_cast<int>(reader->lengthInSamples);

    if (numSamples <= 0)
        return false;

    juce::AudioBuffer<float> buffer(static_cast<int>(reader->numChannels), numSamples);
    reader->read(&buffer, 0, numSamples, 0, true, true);

    destFile.deleteFile();

    std::unique_ptr<juce::OutputStream> outputStream(destFile.createOutputStream());

    if (outputStream == nullptr)
        return false;

    juce::WavAudioFormat wavFormat;

    auto writerOptions = juce::AudioFormatWriterOptions{}
    .withSampleRate(reader->sampleRate)
    .withNumChannels(static_cast<int>(reader->numChannels))
    .withBitsPerSample(static_cast<int>(reader->bitsPerSample));

    auto writer = wavFormat.createWriterFor(outputStream, writerOptions);

    if (writer == nullptr)
        return false;

    return writer->writeFromAudioSampleBuffer(buffer, 0, numSamples);
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

juce::File AudioEngine::getCurrentFile() const
{
    return currentAudioFile;
}