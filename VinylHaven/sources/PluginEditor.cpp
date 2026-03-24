#include "PluginEditor.h"

SamplerAudioProcessorEditor::SamplerAudioProcessorEditor(SamplerAudioProcessor& p) :
    AudioProcessorEditor(&p),
    audioProcessor(p)
{
    setLookAndFeel(&samplerLookAndFeel);

    LoadButton.onClick = [&]() {
        audioProcessor.loadFile([this](juce::String info)
        {
            fileInfoLabel.setText(info, juce::dontSendNotification);
        });
    };
    fileInfoLabel.setText("No file loaded", juce::dontSendNotification);
    fileInfoLabel.setJustificationType(juce::Justification::left);
    fileInfoLabel.setColour(juce::Label::textColourId, juce::Colours::green.withLightness(0.8));

    addAndMakeVisible(&LoadButton);
    addAndMakeVisible(&fileInfoLabel);
    setOpaque(true);
    setSize(800, 400);
}

SamplerAudioProcessorEditor::~SamplerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.97));

    // Painting file display background
    juce::Rectangle<int> fileDispArea = juce::Rectangle<int>(FILE_DISP_X, FILE_DISP_Y, FILE_DISP_WIDTH, FILE_DISP_HEIGHT);
    g.setColour(juce::Colours::grey);
    g.fillRect(fileDispArea.expanded(2));
    g.setColour(juce::Colours::black);
    g.fillRect(fileDispArea);
}

void SamplerAudioProcessorEditor::resized()
{
    LoadButton.setBounds(VINYL_X, VINYL_Y, VINYL_SIZE, VINYL_SIZE);
    fileInfoLabel.setBounds(FILE_DISP_X, FILE_DISP_Y, FILE_DISP_WIDTH, FILE_DISP_HEIGHT);
}