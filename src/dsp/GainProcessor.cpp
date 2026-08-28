#include "GainProcessor.h"

namespace GainProcessor
{
    void applyGainDb(juce::AudioBuffer<float>& buffer, float gainDb)
    {
        buffer.applyGain(juce::Decibels::decibelsToGain(gainDb));
    }

    void normalizePeak(juce::AudioBuffer<float>& buffer, float targetPeakDb)
    {
        float peak = 0.0f;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            peak = juce::jmax(peak, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));

        if (peak <= 0.0f)
            return;

        const float targetLinear = juce::Decibels::decibelsToGain(targetPeakDb);
        buffer.applyGain(targetLinear / peak);
    }
}