#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace FilterResponseCalculator
{
    struct Response
    {
        std::vector<float> magnitudesDb;
        std::vector<float> phasesDegrees;
    };

    Response computeFIRResponse(const std::vector<float>& kernel, double sampleRate, int numPoints = 300);
    Response computeIIRResponse(juce::dsp::IIR::Coefficients<float>::Ptr coefficients,
                                 double sampleRate, int numPoints = 300);

    std::vector<float> computeIIRImpulseResponse(juce::dsp::IIR::Coefficients<float>::Ptr coefficients,
                                                  int numSamples = 200);
}