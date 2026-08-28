#include "IIRFilterProcessor.h"

namespace IIRFilterProcessor
{
    juce::dsp::IIR::Coefficients<float>::Ptr makeCoefficients(double sampleRate, float frequencyHz,
                                                                float q, FilterType type)
    {
        const float nyquist = static_cast<float>(sampleRate / 2.0);
        frequencyHz = juce::jlimit(1.0f, nyquist - 1.0f, frequencyHz);
        q = juce::jmax(0.1f, q);

        switch (type)
        {
            case FilterType::lowPass:  return juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, frequencyHz, q);
            case FilterType::highPass: return juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, frequencyHz, q);
            case FilterType::bandPass: return juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequencyHz, q);
            case FilterType::notch:    return juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, frequencyHz, q);
        }

        return juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, frequencyHz, q);
    }

    void applyFilter(juce::AudioBuffer<float>& buffer, double sampleRate,
                      float frequencyHz, float q, FilterType type)
    {
        auto coeffs = makeCoefficients(sampleRate, frequencyHz, q, type);

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            juce::dsp::IIR::Filter<float> filter;
            filter.coefficients = coeffs;
            filter.reset();

            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                data[i] = filter.processSample(data[i]);
        }
    }
}