#include "DetailsView.h"

DetailsView::DetailsView(juce::AudioFormatManager& formatManagerToUse)
    : formatManager(formatManagerToUse)
{
}

juce::String DetailsView::formatDuration(double seconds) const
{
    const int totalSeconds = static_cast<int>(seconds);
    const int minutes = totalSeconds / 60;
    const int secs = totalSeconds % 60;
    const int millis = static_cast<int>((seconds - totalSeconds) * 1000.0);

    return juce::String::formatted("%02d:%02d.%03d", minutes, secs, millis);
}

juce::String DetailsView::formatFileSize(juce::int64 bytes) const
{
    if (bytes < 1024)
        return juce::String(bytes) + " B";

    if (bytes < 1024 * 1024)
        return juce::String(bytes / 1024.0, 1) + " KB";

    return juce::String(bytes / (1024.0 * 1024.0), 2) + " MB";
}

void DetailsView::setFile(const juce::File& file)
{
    rows.clear();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader == nullptr)
    {
        hasFileLoaded = false;
        repaint();
        return;
    }

    const double lengthInSeconds = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

    rows.push_back({ "File name",       file.getFileName() });
    rows.push_back({ "Format",          reader->getFormatName() });
    rows.push_back({ "Sample rate",     juce::String(reader->sampleRate, 0) + " Hz" });
    rows.push_back({ "Bit depth",       juce::String(reader->bitsPerSample) + " bit" });
    rows.push_back({ "Channels",        juce::String(reader->numChannels)
                                             + (reader->numChannels == 1 ? " (Mono)"
                                                : reader->numChannels == 2 ? " (Stereo)" : "") });
    rows.push_back({ "Duration",        formatDuration(lengthInSeconds) });
    rows.push_back({ "Total samples",   juce::String(reader->lengthInSamples) });
    rows.push_back({ "File size",       formatFileSize(file.getSize()) });

    const auto bitrateKbps = static_cast<int>(
        (reader->sampleRate * reader->bitsPerSample * reader->numChannels) / 1000.0);
    rows.push_back({ "Bitrate (approx)", juce::String(bitrateKbps) + " kbps" });

    rows.push_back({ "Floating point",  reader->usesFloatingPointData ? "Yes" : "No" });

    for (const auto& key : reader->metadataValues.getAllKeys())
    {
        const auto value = reader->metadataValues.getValue(key, {});
        if (value.isNotEmpty())
            rows.push_back({ key, value });
    }

    hasFileLoaded = true;
    repaint();
}

void DetailsView::clear()
{
    rows.clear();
    hasFileLoaded = false;
    repaint();
}

void DetailsView::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds);

    if (!hasFileLoaded)
    {
        g.setColour(juce::Colours::grey);
        g.drawText("No file loaded", bounds, juce::Justification::centred);
        return;
    }

    auto content = bounds.reduced(16);

    constexpr int rowHeight = 24;
    constexpr int labelWidth = 160;

    juce::Font labelFont(juce::FontOptions(15.0f, juce::Font::bold));
    juce::Font valueFont(juce::FontOptions(15.0f));

    for (const auto& row : rows)
    {
        auto rowBounds = content.removeFromTop(rowHeight);

        g.setFont(labelFont);
        g.setColour(juce::Colours::lightgrey);
        g.drawText(row.label, rowBounds.removeFromLeft(labelWidth), juce::Justification::centredLeft);

        g.setFont(valueFont);
        g.setColour(juce::Colours::white);
        g.drawText(row.value, rowBounds, juce::Justification::centredLeft);
    }
}

void DetailsView::resized()
{
}