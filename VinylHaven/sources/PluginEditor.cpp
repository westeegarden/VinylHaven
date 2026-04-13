#include "PluginEditor.h"

juce::Rectangle<int> SamplerAudioProcessorEditor::padRect(int index) const
{
    // Row 0: pads 0-3, Row 1: pads 4-7
    const int col = index % 4;
    const int row = 1 - (index / 4);

    const int padXPositions[4] = { PAD_BOUNDS_X, PAD_2_X, PAD_3_X, PAD_4_X };
    const int padYPositions[2] = { PAD_BOUNDS_Y, PAD_ROW_2_Y };

    return { padXPositions[col], padYPositions[row], PAD_HEIGHT, PAD_HEIGHT };
};

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

    // Knobs and sliders
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
    hiCutKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 17);
    hiCutKnob.setNumDecimalPlacesToDisplay(0);
    hiCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "HICUT", hiCutKnob);
    addAndMakeVisible(&hiCutKnob);

    pitchSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 17);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "PITCH", pitchSlider);
    addAndMakeVisible(&pitchSlider);

    // Pad timestamp editors
    for (int i = 0; i < 8; ++i)
    {
        padEditors[i].setTimestamp(audioProcessor.padTimestamps[i]);
        padEditors[i].onTimestampChanged = [this, i](double newSeconds)
        {
            audioProcessor.padTimestamps[i] = newSeconds;
            audioProcessor.rebuildPadSounds();
        };
        addAndMakeVisible(padEditors[i]);
    }

    setOpaque(true);
    setSize(800, 420);
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

    // Knob and slider labels
    juce::Rectangle<int> crackleLabel(FILE_DISP_X + KNOB_MARGIN - 10, KNOB_Y - 30, LABEL_WIDTH, LABEL_HEIGHT);
    juce::Rectangle<int> locutLabel(FILE_DISP_X + KNOB_SIZE + KNOB_MARGIN * 2 - 10, KNOB_Y - 30, LABEL_WIDTH, LABEL_HEIGHT);
    juce::Rectangle<int> hicutLabel(FILE_DISP_X + KNOB_SIZE * 2 + KNOB_MARGIN * 3 - 10, KNOB_Y - 30, LABEL_WIDTH, LABEL_HEIGHT);
    juce::Rectangle<int> pitchLabel(VINYL_X, KNOB_Y - 30, LABEL_WIDTH, LABEL_HEIGHT);
    juce::Font font("Bauhaus 93", crackleLabel.toFloat().getHeight(), juce::Font::plain);
    g.setColour(juce::Colours::grey);
    g.setFont(font);

    g.drawFittedText("CRACKLE", crackleLabel, juce::Justification::centred, 1);
    g.drawFittedText("LOCUT", locutLabel, juce::Justification::centred, 1);
    g.drawFittedText("HICUT", hicutLabel, juce::Justification::centred, 1);
    g.drawFittedText("PITCH", pitchLabel, juce::Justification::centred, 1);

}

void SamplerAudioProcessorEditor::resized()
{
    // File loading
    LoadButton.setBounds(VINYL_X, VINYL_Y, VINYL_SIZE, VINYL_SIZE);
    fileInfoLabel.setBounds(FILE_DISP_X, FILE_DISP_Y, FILE_DISP_WIDTH, FILE_DISP_HEIGHT);

    // Knobs and sliders
    crackleKnob.setBounds(FILE_DISP_X + KNOB_MARGIN, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 25);
    loCutKnob.setBounds(FILE_DISP_X + KNOB_SIZE + KNOB_MARGIN * 2, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 25);
    hiCutKnob.setBounds(FILE_DISP_X + KNOB_SIZE * 2 + KNOB_MARGIN * 3, KNOB_Y, KNOB_SIZE, KNOB_SIZE + 25);
    pitchSlider.setBounds(VINYL_X, KNOB_Y, VINYL_SIZE, KNOB_SIZE + 25);

    // Timestamp editors: sit in the top portion of each pad
    const int editorH = 22;
    const int editorW = PAD_HEIGHT - 10;
    const int editorYOffset = 8;  // pixels from top of pad

    for (int i = 0; i < NUM_PADS; ++i)
    {
        auto pad = padRect(i);
        padEditors[i].setBounds(pad.getX() + 5,
                                pad.getY() + editorYOffset,
                                editorW,
                                editorH);
    }
}