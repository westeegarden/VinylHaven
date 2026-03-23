#include "PluginEditor.h"

SamplerAudioProcessorEditor::SamplerAudioProcessorEditor(SamplerAudioProcessor& p) :
    AudioProcessorEditor(&p),
    audioProcessor(p)
{
    LoadButton.onClick = [&]() { audioProcessor.loadFile(); };
    addAndMakeVisible(&LoadButton);
    setOpaque(true);
    setSize(800, 500);
}

SamplerAudioProcessorEditor::~SamplerAudioProcessorEditor()
{
}

void SamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey.darker(0.97));
}

void SamplerAudioProcessorEditor::resized()
{
    LoadButton.setBounds(getWidth() / 2 -50, getHeight() / 2 -50, 100, 100);
}