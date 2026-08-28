#include "FilterResponseCalculator.h"
#include <cmath>

namespace FilterResponseCalculator
{
    namespace
    {
        void unwrapPhaseDegrees(std::vector<float>& phases)
        {
            for (size_t i = 1; i < phases.size(); ++i)
            {
                float delta = phases[i] - phases[i - 1];

                while (delta > 180.0f)
                {
                    phases[i] -= 360.0f;
                    delta -= 360.0f;
                }

                while (delta < -180.0f)
                {
                    phases[i] += 360.0f;
                    delta += 360.0f;
                }
            }
        }
    }

    Response computeFIRResponse(const std::vector<float>& kernel, double sampleRate, int numPoints)
    {
        Response response;
        response.magnitudesDb.reserve(static_cast<size_t>(numPoints));
        response.phasesDegrees.reserve(static_cast<size_t>(numPoints));

        const double nyquist = sampleRate / 2.0;

        for (int i = 0; i < numPoints; ++i)
        {
            const double freq = (nyquist * i) / (numPoints - 1);
            const double omega = 2.0 * juce::MathConstants<double>::pi * freq / sampleRate;

            double real = 0.0;
            double imag = 0.0;

            for (size_t n = 0; n < kernel.size(); ++n)
            {
                const double angle = omega * static_cast<double>(n);
                real += kernel[n] * std::cos(angle);
                imag -= kernel[n] * std::sin(angle);
            }

            const double magnitude = std::sqrt(real * real + imag * imag);
            const double magnitudeDb = 20.0 * std::log10(juce::jmax(magnitude, 1.0e-6));
            const double phaseDegrees = std::atan2(imag, real) * 180.0 / juce::MathConstants<double>::pi;

            response.magnitudesDb.push_back(static_cast<float>(magnitudeDb));
            response.phasesDegrees.push_back(static_cast<float>(phaseDegrees));
        }

        unwrapPhaseDegrees(response.phasesDegrees);

        return response;
    }

    Response computeIIRResponse(juce::dsp::IIR::Coefficients<float>::Ptr coefficients,
                                 double sampleRate, int numPoints)
    {
        Response response;
        response.magnitudesDb.reserve(static_cast<size_t>(numPoints));
        response.phasesDegrees.reserve(static_cast<size_t>(numPoints));

        const double nyquist = sampleRate / 2.0;

        for (int i = 0; i < numPoints; ++i)
        {
            const double freq = juce::jmax(1.0, (nyquist * i) / (numPoints - 1));

            const double magnitude = coefficients->getMagnitudeForFrequency(freq, sampleRate);
            const double phaseRadians = coefficients->getPhaseForFrequency(freq, sampleRate);

            const double magnitudeDb = 20.0 * std::log10(juce::jmax(magnitude, 1.0e-6));
            const double phaseDegrees = phaseRadians * 180.0 / juce::MathConstants<double>::pi;

            response.magnitudesDb.push_back(static_cast<float>(magnitudeDb));
            response.phasesDegrees.push_back(static_cast<float>(phaseDegrees));
        }

        unwrapPhaseDegrees(response.phasesDegrees);

        return response;
    }

    std::vector<float> computeIIRImpulseResponse(juce::dsp::IIR::Coefficients<float>::Ptr coefficients,
                                                  int numSamples)
    {
        juce::dsp::IIR::Filter<float> filter;
        filter.coefficients = coefficients;
        filter.reset();

        std::vector<float> output(static_cast<size_t>(numSamples), 0.0f);

        for (int i = 0; i < numSamples; ++i)
        {
            const float input = (i == 0) ? 1.0f : 0.0f;
            output[static_cast<size_t>(i)] = filter.processSample(input);
        }

        return output;
    }
}