#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include "FIRFilterProcessor.h"
#include "IIRFilterProcessor.h"

class ProcessingEngine
{
public:
    explicit ProcessingEngine(juce::AudioFormatManager& formatManagerToUse);

    juce::File applyGain(const juce::File& sourceFile, float gainDb);
    juce::File applyNormalize(const juce::File& sourceFile, float targetPeakDb = 0.0f);
    juce::File applyFIRFilter(const juce::File& sourceFile, float cutoffHz, FIRFilterProcessor::FilterType type);
    juce::File applyConvolution(const juce::File& sourceFile, const juce::File& impulseResponseFile, float wetDryMix);
    juce::File applyIIRFilter(const juce::File& sourceFile, float frequencyHz, float q, IIRFilterProcessor::FilterType type);

private:
    bool readFileToBuffer(const juce::File& file, juce::AudioBuffer<float>& buffer,
                           double& sampleRate, int& bitsPerSample, int& numChannels);

    juce::File writeBufferToTempFile(const juce::AudioBuffer<float>& buffer,
                                      double sampleRate, int bitsPerSample, int numChannels);

    juce::AudioFormatManager& formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessingEngine)
};