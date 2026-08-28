#include "ConvolutionProcessor.h"

namespace ConvolutionProcessor
{
    void applyConvolution(juce::AudioBuffer<float>& buffer, double sampleRate,
                           const juce::File& impulseResponseFile, float wetDryMix)
    {
        constexpr int blockSize = 4096;
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        juce::dsp::Convolution convolution;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(blockSize);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

        convolution.prepare(spec);
        convolution.loadImpulseResponse(impulseResponseFile,
            numChannels > 1 ? juce::dsp::Convolution::Stereo::yes : juce::dsp::Convolution::Stereo::no,
            juce::dsp::Convolution::Trim::yes,
            0,
            juce::dsp::Convolution::Normalise::yes);

        juce::AudioBuffer<float> dryCopy;
        dryCopy.makeCopyOf(buffer);

        for (int start = 0; start < numSamples; start += blockSize)
        {
            const int len = juce::jmin(blockSize, numSamples - start);

            juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(),
                                                static_cast<size_t>(numChannels),
                                                static_cast<size_t>(start),
                                                static_cast<size_t>(len));

            juce::dsp::ProcessContextReplacing<float> context(block);
            convolution.process(context);
        }

        const float dryLevel = 1.0f - wetDryMix;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* wet = buffer.getWritePointer(ch);
            auto* dry = dryCopy.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
                wet[i] = (i < dryCopy.getNumSamples() ? dry[i] * dryLevel : 0.0f) + wet[i] * wetDryMix;
        }
    }
}