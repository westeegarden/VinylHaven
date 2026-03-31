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

    crackleKnob.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    crackleKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 17);
    crackleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "CRACKLE", crackleKnob);
    crackleKnob.setRange(0.0, 10.0, 1.0);
    addAndMakeVisible(&crackleKnob);

    loCutKnob.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    loCutKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 17);
    loCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "LOCUT", loCutKnob);
    addAndMakeVisible(&loCutKnob);

    hiCutKnob.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    hiCutKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 17);
    hiCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "HICUT", hiCutKnob);
    addAndMakeVisible(&hiCutKnob);

    pitchSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 17);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PITCH", pitchSlider);
    addAndMakeVisible(&pitchSlider);

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
    juce::Rectangle<int> fileDispArea(FILE_DISP_X, FILE_DISP_Y, FILE_DISP_WIDTH, FILE_DISP_HEIGHT);
    g.setColour(juce::Colours::grey);
    g.fillRect(fileDispArea.expanded(2));
    g.setColour(juce::Colours::black);
    g.fillRect(fileDispArea);

    // Pad area
    juce::Rectangle<int> padsBounds(PAD_BOUNDS_X, PAD_BOUNDS_Y, PAD_BOUNDS_WIDTH, PAD_BOUNDS_HEIGHT);


    juce::Rectangle<int> pad1(PAD_BOUNDS_X, PAD_BOUNDS_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad2(PAD_2_X, PAD_BOUNDS_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad3(PAD_3_X, PAD_BOUNDS_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad4(PAD_4_X, PAD_BOUNDS_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad5(PAD_BOUNDS_X, PAD_ROW_2_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad6(PAD_2_X, PAD_ROW_2_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad7(PAD_3_X, PAD_ROW_2_Y, PAD_HEIGHT, PAD_HEIGHT);
    juce::Rectangle<int> pad8(PAD_4_X, PAD_ROW_2_Y, PAD_HEIGHT, PAD_HEIGHT);
    g.setColour(juce::Colours::orange);
    g.fillRoundedRectangle(pad1.toFloat(), 10);
    g.fillRoundedRectangle(pad2.toFloat(), 10);
    g.fillRoundedRectangle(pad3.toFloat(), 10);
    g.fillRoundedRectangle(pad4.toFloat(), 10);
    g.fillRoundedRectangle(pad5.toFloat(), 10);
    g.fillRoundedRectangle(pad6.toFloat(), 10);
    g.fillRoundedRectangle(pad7.toFloat(), 10);
    g.fillRoundedRectangle(pad8.toFloat(), 10);
    g.setColour(juce::Colours::grey);
    g.fillRoundedRectangle(pad1.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad2.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad3.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad4.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad5.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad6.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad7.toFloat().reduced(2), 10);
    g.fillRoundedRectangle(pad8.toFloat().reduced(2), 10);
}

void SamplerAudioProcessorEditor::resized()
{
    LoadButton.setBounds(VINYL_X, VINYL_Y, VINYL_SIZE, VINYL_SIZE);
    fileInfoLabel.setBounds(FILE_DISP_X, FILE_DISP_Y, FILE_DISP_WIDTH, FILE_DISP_HEIGHT);

    crackleKnob.setBounds(FILE_DISP_X + KNOB_MARGIN, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 30);
    loCutKnob.setBounds(FILE_DISP_X + KNOB_SIZE + KNOB_MARGIN * 2, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 30);
    hiCutKnob.setBounds(FILE_DISP_X + KNOB_SIZE * 2 + KNOB_MARGIN * 3, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 30);
    pitchSlider.setBounds(VINYL_X, KNOB_Y, VINYL_SIZE, KNOB_SIZE + 20);
}