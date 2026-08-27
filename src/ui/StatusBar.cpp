#include "StatusBar.h"

StatusBar::StatusBar()
{
    messageLabel.setJustificationType(juce::Justification::centredLeft);
    messageLabel.setFont(juce::Font(juce::FontOptions(14.0f)));
    addAndMakeVisible(messageLabel);

    setMessage("Hish is ready for use");
}

void StatusBar::setMessage(const juce::String& message)
{
    messageLabel.setText(message, juce::dontSendNotification);
}

void StatusBar::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(0.1f));

    g.setColour(getLookAndFeel().findColour(juce::ComboBox::outlineColourId));
    g.drawLine(0.0f, 0.0f, static_cast<float>(getWidth()), 0.0f, 1.0f);
}

void StatusBar::resized()
{
    messageLabel.setBounds(getLocalBounds().reduced(8, 0));
}