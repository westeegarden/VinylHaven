#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "PluginProcessor.h"

class SamplerLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::Rectangle<float> buttonBounds = button.getLocalBounds().toFloat();

            // Drawing the turntable
            g.setColour(juce::Colours::grey.darker(0.3));
            g.fillEllipse(buttonBounds);
            g.setColour(juce::Colours::black);
            g.fillEllipse(buttonBounds.reduced(5));
            g.setColour(juce::Colours::orange.darker(0.6));
            g.fillEllipse(buttonBounds.reduced(85));
            g.setColour(juce::Colours::darkolivegreen.darker(0.9));
            g.fillEllipse(buttonBounds.reduced(87));
            g.setColour(juce::Colours::black);
            g.fillEllipse(buttonBounds.reduced(120));

            //Drawing tonearm
            float baseSize = 40;
            juce::Rectangle<float> tonearmBase(buttonBounds.getX() + buttonBounds.getWidth() - baseSize,
                                                buttonBounds.getY(), baseSize, baseSize);
            g.setColour(juce::Colours::black);
            g.fillEllipse(tonearmBase);
            g.setColour(juce::Colours::grey.darker(0.3));
            g.fillEllipse(tonearmBase.reduced(2));

            juce::Rectangle<float> tonearm(buttonBounds.getX() + buttonBounds.getWidth() / 2, buttonBounds.getY() + 15, buttonBounds.getWidth() / 2 - 15, 10);
            g.setColour(juce::Colours::grey);
            g.fillRoundedRectangle(tonearm, 5);

            juce::Rectangle<int> cartridge(buttonBounds.getX() + buttonBounds.getWidth() / 2 - 10, buttonBounds.getY() + 10, 30, 20);
            g.setColour(juce::Colours::black.withLightness(0.1));
            g.fillRect(cartridge);
            g.setColour(juce::Colours::grey.darker(0.3));
            g.fillRect(cartridge.reduced(2));
        }

        void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
            juce::Rectangle<int> buttonBounds = button.getLocalBounds().translated(20, 0);
            juce::Font font("Bauhaus 93", 25, juce::Font::plain);
            g.setFont(font);
            g.setColour(juce::Colours::darkgrey);
            g.drawFittedText("LOAD                 SAMPLE", buttonBounds, juce::Justification::left, 1);
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
            float radius = width / 2;
            float centreX = x + radius;
            float centreY = y + radius;
            float rx = centreX - radius;
            float ry = centreY - radius;
            float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

            juce::Rectangle<float> dialArea(x, y, width, width);

            g.setColour(juce::Colours::grey);
            g.fillEllipse(dialArea);
            g.setColour(juce::Colours::black);
            g.fillEllipse(dialArea.reduced(1));

            g.setColour(juce::Colours::white);

            juce::Path dialTick;
            auto pointerLength = radius * 0.5f;
            auto pointerThickness = 2.0f;
            dialTick.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);

            g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        }

        void drawLinearSlider (juce::Graphics &g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider &s) override {
            juce::Rectangle<float> sliderArea(x, y, width, height);
            juce::Rectangle<float> slot(x, y + (height / 2), width, 5);
            juce::Rectangle<float> thumb(sliderPos - 8, y, 16, height);
            juce::Rectangle<float> thumbLine(sliderPos - 1, y + 1, 2, height - 2);
            g.setColour(juce::Colours::black);
            g.fillRoundedRectangle(slot, 5);
            g.setColour(juce::Colours::grey);
            g.fillRect(thumb);
            g.setColour(juce::Colours::black);
            g.fillRect(thumb.reduced(1));
            g.setColour(juce::Colours::white);
            g.fillRect(thumbLine);

        }
};

class PadTimestampEditor : public juce::Component
{
public:
    std::function<void(double)> onTimestampChanged;

    PadTimestampEditor()
    {
        minutesBox.setInputRestrictions(2, "0123456789");
        secondsBox.setInputRestrictions(5, "0123456789.");

        minutesBox.setJustification(juce::Justification::centred);
        secondsBox.setJustification(juce::Justification::centred);

        minutesBox.setText("0",  juce::dontSendNotification);
        secondsBox.setText("00", juce::dontSendNotification);

        auto styleBox = [](juce::TextEditor& te)
        {
            te.setColour(juce::TextEditor::backgroundColourId,    juce::Colours::black.withAlpha(0.6f));
            te.setColour(juce::TextEditor::textColourId,          juce::Colours::white);
            te.setColour(juce::TextEditor::outlineColourId,       juce::Colours::transparentBlack);
            te.setColour(juce::TextEditor::focusedOutlineColourId,juce::Colours::orange);
            //te.setFont(juce::Font("Bauhaus 93", 13.0f, juce::Font::plain));
        };

        styleBox(minutesBox);
        styleBox(secondsBox);

        auto commitOnFocusLoss = [this](juce::TextEditor&) { commitValue(); };
        minutesBox.onFocusLost = [this]() { commitValue(); };
        secondsBox.onFocusLost = [this]() { commitValue(); };
        minutesBox.onReturnKey = [this]() { commitValue(); };
        secondsBox.onReturnKey = [this]() { commitValue(); };

        addAndMakeVisible(minutesBox);
        addAndMakeVisible(colon);
        addAndMakeVisible(secondsBox);

        colon.setText(":", juce::dontSendNotification);
        colon.setJustificationType(juce::Justification::centred);
        colon.setColour(juce::Label::textColourId, juce::Colours::white);
        //colon.setFont(juce::Font("Bauhaus 93", 13.0f, juce::Font::plain));
        colon.setInterceptsMouseClicks(false, false);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        int colonW = 8;
        int fieldW = (area.getWidth() - colonW) / 2;
        minutesBox.setBounds(area.removeFromLeft(fieldW));
        area.removeFromLeft(2);
        colon.setBounds(area.removeFromLeft(colonW - 2));
        secondsBox.setBounds(area);
    }

    void setTimestamp(double seconds)
    {
        int mins = (int)(seconds / 60.0);
        double secs = seconds - mins * 60.0;
        minutesBox.setText(juce::String(mins), juce::dontSendNotification);
        secondsBox.setText(juce::String(secs, 2), juce::dontSendNotification);
    }

    double getTimestampSeconds() const
    {
        int mins = minutesBox.getText().getIntValue();
        double secs = juce::jlimit(0.0, 59.99, secondsBox.getText().getDoubleValue());
        return (double)(mins * 60) + secs;
    }

private:
    void commitValue()
    {
        // Clamp seconds display to 00-59
        double secs = juce::jlimit(0.0, 59.99, secondsBox.getText().getDoubleValue());
        secondsBox.setText(juce::String(secs, 2), juce::dontSendNotification);

        if (onTimestampChanged)
            onTimestampChanged(getTimestampSeconds());
    }

    juce::TextEditor minutesBox, secondsBox;
    juce::Label colon;
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
    const int FILE_DISP_X = leftEdge + 30;
    const int FILE_DISP_Y = topEdge + 30;
    const int FILE_DISP_WIDTH = 420;
    const int FILE_DISP_HEIGHT = 20;

    // Pads constants
    const int NUM_PADS = 8;
    const int PAD_BOUNDS_X = leftEdge + 30;
    const int PAD_BOUNDS_Y = topEdge + 70;
    const int PAD_BOUNDS_WIDTH = 460;
    const int PAD_BOUNDS_HEIGHT = 200;
    const int PAD_MARGIN = 15;
    const int PAD_HEIGHT = PAD_BOUNDS_HEIGHT / 2 - 5;
    const int PAD_2_X = PAD_BOUNDS_X + PAD_HEIGHT + PAD_MARGIN;
    const int PAD_3_X = PAD_BOUNDS_X + PAD_HEIGHT*2 + PAD_MARGIN*2;
    const int PAD_4_X = PAD_BOUNDS_X + PAD_HEIGHT*3 + PAD_MARGIN*3;
    const int PAD_ROW_2_Y = PAD_BOUNDS_Y + PAD_HEIGHT + PAD_MARGIN;

    // Knob and slider constants
    const int KNOB_SIZE = PAD_HEIGHT - 40;
    const int KNOB_Y = FILE_DISP_Y + 290;
    const int KNOB_MARGIN = (PAD_BOUNDS_WIDTH - KNOB_SIZE * 3) / 4;

    // Label constants
    const int LABEL_HEIGHT = 25;
    const int LABEL_WIDTH = 70;

    explicit SamplerAudioProcessorEditor(SamplerAudioProcessor&);
    ~SamplerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SamplerLookAndFeel samplerLookAndFeel;
    juce::TextButton LoadButton { "Load" };
    juce::Label fileInfoLabel;

    juce::Slider crackleKnob;
    juce::Slider hiCutKnob;
    juce::Slider loCutKnob;
    juce::Slider pitchSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crackleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;

    PadTimestampEditor padEditors[8];
    juce::Rectangle<int> padRect(int index) const;

    SamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessorEditor)
};
