#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class AudioEngine
{
public:
    explicit AudioEngine(juce::AudioFormatManager& formatManagerToUse);
    ~AudioEngine();

    bool loadFile(const juce::File& file);
    bool exportToFile(const juce::File& destFile) const;

    void play();
    void pause();
    void stop();
    void setPosition(double positionInSeconds);
    void setGain(float newGain);

    bool isPlaying() const;
    double getLengthInSeconds() const;
    double getCurrentPosition() const;
    double getSampleRate() const;
    int getNumChannels() const;

    juce::File getCurrentFile() const;

private:
    juce::AudioFormatManager& formatManager;

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    juce::File currentAudioFile;
    double lastSampleRate = 44100.0;
    int lastNumChannels = 2;
    int lastBitsPerSample = 16;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};