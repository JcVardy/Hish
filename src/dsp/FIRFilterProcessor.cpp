#include "FIRFilterProcessor.h"
#include <cmath>

namespace FIRFilterProcessor
{
    namespace
    {
        std::vector<float> designLowpassKernel(int numTaps, double sampleRate, float cutoffHz)
        {
            std::vector<float> kernel(static_cast<size_t>(numTaps));
            const int M = numTaps - 1;
            const double fc = cutoffHz / sampleRate;

            for (int n = 0; n < numTaps; ++n)
            {
                const double shifted = n - M / 2.0;

                double sincValue;
                if (std::abs(shifted) < 1e-9)
                    sincValue = 2.0 * fc;
                else
                    sincValue = std::sin(2.0 * juce::MathConstants<double>::pi * fc * shifted)
                                / (juce::MathConstants<double>::pi * shifted);

                const double window = 0.42
                    - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * n / M)
                    + 0.08 * std::cos(4.0 * juce::MathConstants<double>::pi * n / M);

                kernel[static_cast<size_t>(n)] = static_cast<float>(sincValue * window);
            }

            double sum = 0.0;
            for (auto v : kernel)
                sum += v;

            if (sum != 0.0)
                for (auto& v : kernel)
                    v = static_cast<float>(v / sum);

            return kernel;
        }

        std::vector<float> spectralInvert(std::vector<float> kernel)
        {
            for (auto& v : kernel)
                v = -v;

            const size_t centre = kernel.size() / 2;
            kernel[centre] += 1.0f;

            return kernel;
        }
    }

    std::vector<float> designKernel(double sampleRate, float cutoffHz, FilterType type, int numTaps)
    {
        if (numTaps % 2 == 0)
            ++numTaps;

        const float nyquist = static_cast<float>(sampleRate / 2.0);
        cutoffHz = juce::jlimit(1.0f, nyquist - 1.0f, cutoffHz);

        auto kernel = designLowpassKernel(numTaps, sampleRate, cutoffHz);

        if (type == FilterType::highPass)
            kernel = spectralInvert(kernel);

        return kernel;
    }

    void applyFilter(juce::AudioBuffer<float>& buffer, double sampleRate,
                      float cutoffHz, FilterType type, int numTaps)
    {
        auto kernel = designKernel(sampleRate, cutoffHz, type, numTaps);

        juce::dsp::FIR::Coefficients<float>::Ptr coeffs =
            new juce::dsp::FIR::Coefficients<float>(kernel.data(), kernel.size());

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            juce::dsp::FIR::Filter<float> filter;
            filter.coefficients = coeffs;
            filter.reset();

            auto* data = buffer.getWritePointer(ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                data[i] = filter.processSample(data[i]);
        }
    }
}