#pragma once

#include <juce_dsp/juce_dsp.h>

namespace IIRFilterProcessor
{
    enum class FilterType
    {
        lowPass,
        highPass,
        bandPass,
        notch
    };

    juce::dsp::IIR::Coefficients<float>::Ptr makeCoefficients(double sampleRate, float frequencyHz,
                                                                float q, FilterType type);

    void applyFilter(juce::AudioBuffer<float>& buffer, double sampleRate,
                      float frequencyHz, float q, FilterType type);
}