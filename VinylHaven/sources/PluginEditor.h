#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "PluginProcessor.h"

class SamplerLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::Rectangle<float> buttonBounds = button.getLocalBounds().toFloat();

            // Drawing the turntable
            g.setColour(juce::Colours::grey);
            g.fillEllipse(buttonBounds);
            g.setColour(juce::Colours::black);
            g.fillEllipse(buttonBounds.reduced(5));
            g.setColour(juce::Colours::orange.darker(0.6));
            g.fillEllipse(buttonBounds.reduced(85));
            g.setColour(juce::Colours::darkolivegreen.darker(0.9));
            g.fillEllipse(buttonBounds.reduced(87));
            g.setColour(juce::Colours::black);
            g.fillEllipse(buttonBounds.reduced(120));
        }
};

class SamplerAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:

    juce::Rectangle<int> bounds = getLocalBounds();
    const int leftEdge = bounds.getX();
    const int topEdge = bounds.getY();
    const int x_center = bounds.getWidth() / 2;
    const int y_center = bounds.getHeight() / 2;

    // Vinyl constants
    const int VINYL_X = x_center + 520;
    const int VINYL_Y = y_center + 20;
    const int VINYL_SIZE = 250;

    // File display constants
    const int FILE_DISP_X = leftEdge + 20;
    const int FILE_DISP_Y = topEdge + 30;
    const int FILE_DISP_WIDTH = 300;
    const int FILE_DISP_HEIGHT = 20;

    explicit SamplerAudioProcessorEditor(SamplerAudioProcessor&);
    ~SamplerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SamplerLookAndFeel samplerLookAndFeel;
    juce::TextButton LoadButton { "Load" };
    juce::Label fileInfoLabel;

    SamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessorEditor)
};
