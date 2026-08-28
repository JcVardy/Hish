#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace FIRFilterProcessor
{
    enum class FilterType
    {
        lowPass,
        highPass
    };

    std::vector<float> designKernel(double sampleRate, float cutoffHz, FilterType type, int numTaps = 101);

    void applyFilter(juce::AudioBuffer<float>& buffer, double sampleRate,
                      float cutoffHz, FilterType type, int numTaps = 101);
}