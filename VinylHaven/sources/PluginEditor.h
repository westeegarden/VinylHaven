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

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override {
            /*float diameter = 0;
            if (width < height)
            {
                diameter = width;
            }
            else
            {
                diameter = height;
            }
            */

            //float diameter = 84;
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
    const int KNOB_Y = FILE_DISP_Y + 270;
    const int KNOB_MARGIN = (PAD_BOUNDS_WIDTH - KNOB_SIZE * 3) / 4;

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

    SamplerAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessorEditor)
};
