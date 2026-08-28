#include "ProcessingEngine.h"
#include "GainProcessor.h"
#include "ConvolutionProcessor.h"

ProcessingEngine::ProcessingEngine(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
}

bool ProcessingEngine::readFileToBuffer(const juce::File& file, juce::AudioBuffer<float>& buffer,
                                         double& sampleRate, int& bitsPerSample, int& numChannels)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
        return false;

    const auto numSamples = static_cast<int>(reader->lengthInSamples);

    if (numSamples <= 0)
        return false;

    buffer.setSize(static_cast<int>(reader->numChannels), numSamples);
    reader->read(&buffer, 0, numSamples, 0, true, true);

    sampleRate = reader->sampleRate;
    bitsPerSample = static_cast<int>(reader->bitsPerSample);
    numChannels = static_cast<int>(reader->numChannels);

    return true;
}

juce::File ProcessingEngine::writeBufferToTempFile(const juce::AudioBuffer<float>& buffer,
                                                    double sampleRate, int bitsPerSample, int numChannels)
{
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    auto tempFile = tempDir.getChildFile("Hish_processed_"
        + juce::String(juce::Time::getMillisecondCounterHiRes(), 0) + ".wav");

    tempFile.deleteFile();

    std::unique_ptr<juce::OutputStream> outputStream(tempFile.createOutputStream());

    if (outputStream == nullptr)
        return {};

    juce::WavAudioFormat wavFormat;

    auto writerOptions = juce::AudioFormatWriterOptions{}
        .withSampleRate(sampleRate)
        .withNumChannels(numChannels)
        .withBitsPerSample(bitsPerSample);

    auto writer = wavFormat.createWriterFor(outputStream, writerOptions);

    if (writer == nullptr)
        return {};

    if (!writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples()))
        return {};

    return tempFile;
}

juce::File ProcessingEngine::applyGain(const juce::File& sourceFile, float gainDb)
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bitsPerSample = 16;
    int numChannels = 2;

    if (!readFileToBuffer(sourceFile, buffer, sampleRate, bitsPerSample, numChannels))
        return {};

    GainProcessor::applyGainDb(buffer, gainDb);

    return writeBufferToTempFile(buffer, sampleRate, bitsPerSample, numChannels);
}

juce::File ProcessingEngine::applyNormalize(const juce::File& sourceFile, float targetPeakDb)
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bitsPerSample = 16;
    int numChannels = 2;

    if (!readFileToBuffer(sourceFile, buffer, sampleRate, bitsPerSample, numChannels))
        return {};

    GainProcessor::normalizePeak(buffer, targetPeakDb);

    return writeBufferToTempFile(buffer, sampleRate, bitsPerSample, numChannels);
}

juce::File ProcessingEngine::applyFIRFilter(const juce::File& sourceFile, float cutoffHz,
                                             FIRFilterProcessor::FilterType type)
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bitsPerSample = 16;
    int numChannels = 2;

    if (!readFileToBuffer(sourceFile, buffer, sampleRate, bitsPerSample, numChannels))
        return {};

    FIRFilterProcessor::applyFilter(buffer, sampleRate, cutoffHz, type);

    return writeBufferToTempFile(buffer, sampleRate, bitsPerSample, numChannels);
}

juce::File ProcessingEngine::applyConvolution(const juce::File& sourceFile, const juce::File& impulseResponseFile,
                                               float wetDryMix)
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bitsPerSample = 16;
    int numChannels = 2;

    if (!readFileToBuffer(sourceFile, buffer, sampleRate, bitsPerSample, numChannels))
        return {};

    // Pad with silence so the reverb tail isn't truncated at the original file's end.
    std::unique_ptr<juce::AudioFormatReader> irReader(formatManager.createReaderFor(impulseResponseFile));

    if (irReader == nullptr)
        return {};

    const int tailSamples = static_cast<int>(irReader->lengthInSamples);
    const int originalLength = buffer.getNumSamples();

    buffer.setSize(numChannels, originalLength + tailSamples, true, true, true);

    ConvolutionProcessor::applyConvolution(buffer, sampleRate, impulseResponseFile, wetDryMix);

    return writeBufferToTempFile(buffer, sampleRate, bitsPerSample, numChannels);
}

juce::File ProcessingEngine::applyIIRFilter(const juce::File& sourceFile, float frequencyHz, float q,
                                             IIRFilterProcessor::FilterType type)
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int bitsPerSample = 16;
    int numChannels = 2;

    if (!readFileToBuffer(sourceFile, buffer, sampleRate, bitsPerSample, numChannels))
        return {};

    IIRFilterProcessor::applyFilter(buffer, sampleRate, frequencyHz, q, type);

    return writeBufferToTempFile(buffer, sampleRate, bitsPerSample, numChannels);
}