#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class AudioEngine
{
public:
    explicit AudioEngine(juce::AudioFormatManager& formatManagerToUse);
    ~AudioEngine();

    bool loadFile(const juce::File& file);

    void play();
    void pause();
    void stop();

    void setGain(float newGain);

    bool isPlaying() const;
    double getLengthInSeconds() const;
    double getCurrentPosition() const;
    double getSampleRate() const;
    int getNumChannels() const;
    void setPosition(double positionInSeconds);

    double lastSampleRate = 44100.0;
    int lastNumChannels = 2;

private:
    juce::AudioFormatManager& formatManager;

    juce::AudioDeviceManager deviceManager;
    juce::AudioSourcePlayer sourcePlayer;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};