#pragma once

#include <juce_dsp/juce_dsp.h>

namespace ConvolutionProcessor
{
    void applyConvolution(juce::AudioBuffer<float>& buffer, double sampleRate,
                           const juce::File& impulseResponseFile, float wetDryMix);
}