#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

class DetailsView : public juce::Component
{
public:
    explicit DetailsView(juce::AudioFormatManager& formatManagerToUse);

    void setFile(const juce::File& file);
    void clear();

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    struct DetailRow
    {
        juce::String label;
        juce::String value;
    };

    juce::String formatDuration(double seconds) const;
    juce::String formatFileSize(juce::int64 bytes) const;

    juce::AudioFormatManager& formatManager;
    std::vector<DetailRow> rows;
    bool hasFileLoaded = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetailsView)
};