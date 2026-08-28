#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace GainProcessor
{
    void applyGainDb(juce::AudioBuffer<float>& buffer, float gainDb);
    void normalizePeak(juce::AudioBuffer<float>& buffer, float targetPeakDb = 0.0f);
}