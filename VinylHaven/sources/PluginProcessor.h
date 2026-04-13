#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "BinaryData.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include <juce_dsp/juce_dsp.h>

#define PITCH_MIN -24.0
#define PITCH_MAX 24.0
#define PITCH_DEFAULT 0.0
#define LOCUT_MIN 0.0
#define LOCUT_MAX 500.0
#define LOCUT_DEFAULT 0.0
#define HICUT_MIN 1000.0
#define HICUT_MAX 20000.0
#define HICUT_DEFAULT 20000.0
#define CRACKLE_MIN 0.0
#define CRACKLE_MAX 10.0
#define CRACKLE_DEFAULT 0.0

static constexpr int NUM_PADS = 8;
static constexpr int PAD_MIDI_START = 36;



class SamplerAudioProcessor final : public juce::AudioProcessor,
                                    public juce::AudioProcessorValueTreeState::Listener
{
public:
    SamplerAudioProcessor();
    ~SamplerAudioProcessor();

    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override { }
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override { }

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override { }
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    juce::AudioProcessorEditor* createEditor() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState apvts;
    juce::String fileInfoString { "No file loaded" };

    // Pad timestamps in seconds
    double padTimestamps[NUM_PADS] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    double fileDurationSeconds { 0.0 };

    void loadFile(std::function<void(juce::String)> callBack = nullptr);

    void rebuildPadSounds();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void clearUnusedOutputChannels(juce::AudioBuffer<float>& buffer) const;
    void updateFilters();

    std::function<void(juce::String)> onLoaded;
    std::unique_ptr<juce::FileChooser> chooser;

    juce::Synthesiser midiPlaybackEngine;
    juce::AudioFormatManager formatManager;
    juce::AudioFormatReader* formatReader { nullptr };

    std::vector<std::unique_ptr<juce::AudioSubsectionReader>> padReaders;
    std::vector<int> padMidiNotes{ 36, 37, 38, 39, 40, 41, 42, 43 };

    std::unique_ptr<juce::AudioBuffer<float>> fileBuffer;
    double fileOriginalSampleRate { 44100.0 };

    // DSP
    using FilterChain = juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Filter<float>
    >;
    FilterChain filterChainL, filterChainR;
    juce::dsp::ProcessSpec processSpec { 44100.0, 512, 1 };

    juce::Random crackleRandom;
    double currentSampleRate { 44100.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessor)
};
