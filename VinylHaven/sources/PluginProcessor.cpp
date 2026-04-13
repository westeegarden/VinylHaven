#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_formats/juce_audio_formats.h>


SamplerAudioProcessor::SamplerAudioProcessor() :
    AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();

    // 8 voices, one per pad
    //for (int i = 0; i < NUM_PADS; ++i)
    midiPlaybackEngine.addVoice(new juce::SamplerVoice());

    apvts.addParameterListener("PITCH", this);
    apvts.addParameterListener("LOCUT",   this);
    apvts.addParameterListener("HICUT",   this);
    apvts.addParameterListener("CRACKLE", this);
}

SamplerAudioProcessor::~SamplerAudioProcessor() {
    formatReader = nullptr;
}

bool SamplerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SamplerAudioProcessor::prepareToPlay(double newSampleRate, int maximumBlockSize)
{
    currentSampleRate = newSampleRate;
    midiPlaybackEngine.setCurrentPlaybackSampleRate(newSampleRate);

    processSpec.sampleRate       = newSampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
    processSpec.numChannels      = 1;

    filterChainL.prepare(processSpec);
    filterChainR.prepare(processSpec);

    updateFilters();
}


void SamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    clearUnusedOutputChannels(buffer);
    midiPlaybackEngine.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply lo/hi cut filters
    {
        auto blockL = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(0);
        auto blockR = juce::dsp::AudioBlock<float>(buffer).getSingleChannelBlock(1);
        juce::dsp::ProcessContextReplacing<float> ctxL(blockL);
        juce::dsp::ProcessContextReplacing<float> ctxR(blockR);
        filterChainL.process(ctxL);
        filterChainR.process(ctxR);
    }

    // Mix in vinyl crackle
    float crackleAmount = apvts.getRawParameterValue("CRACKLE")->load();
    if (crackleAmount > 0.0f)
    {
        float gain = crackleAmount / (float)CRACKLE_MAX * 0.08f;  // max ~8% noise
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                // Band-limited crackle: sum two random values for softer noise
                float noise = (crackleRandom.nextFloat() * 1.5f - 1.0f)
                            * (crackleRandom.nextFloat() * 1.5f - 1.0f);  // multiply for pops
                // Rare loud pops
                if (crackleRandom.nextFloat() > 0.9995f)
                    noise += (crackleRandom.nextFloat() * 2.0f - 1.0f) * 15.0f;

                // Extra rare extra loud pops
                if (crackleRandom.nextFloat() > 0.9999f)
                    noise += (crackleRandom.nextFloat() * 2.0f - 1.0f) * 20.0f;

                data[s] += noise * gain;
            }
        }
    }
}

void SamplerAudioProcessor::clearUnusedOutputChannels(juce::AudioBuffer<float>& buffer) const
{
    for ( auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
}

void SamplerAudioProcessor::parameterChanged(const juce::String& parameterID, float /*newValue*/)
{
    if (parameterID == "PITCH")
        rebuildPadSounds();
    else if (parameterID == "LOCUT" || parameterID == "HICUT")
        updateFilters();
}

void SamplerAudioProcessor::updateFilters()
{
    float loCut  = apvts.getRawParameterValue("LOCUT")->load();
    float hiCut  = apvts.getRawParameterValue("HICUT")->load();

    loCut = juce::jlimit(20.0f,  (float)currentSampleRate * 0.49f, loCut);
    hiCut = juce::jlimit(loCut + 10.0f, (float)currentSampleRate * 0.49f, hiCut);

    auto hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, loCut, 0.707f);
    auto lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (currentSampleRate, hiCut, 0.707f);

    *filterChainL.get<0>().coefficients = *hpCoeffs;
    *filterChainL.get<1>().coefficients = *lpCoeffs;
    *filterChainR.get<0>().coefficients = *hpCoeffs;
    *filterChainR.get<1>().coefficients = *lpCoeffs;
}

void SamplerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void SamplerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* SamplerAudioProcessor::createEditor()
{
    return new SamplerAudioProcessorEditor(*this);
}

void SamplerAudioProcessor::loadFile(std::function<void(juce::String)> callback) {
    onLoaded = callback;
    chooser = std::make_unique<juce::FileChooser>("Please load a file");

    auto flags = juce::FileBrowserComponent::openMode |
                 juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file.existsAsFile())
        {
            formatReader = formatManager.createReaderFor(file);

            // build display string
            long lengthInSeconds = formatReader->lengthInSamples / formatReader->sampleRate;
            int minutes = static_cast<int>(lengthInSeconds / 60);
            int seconds = static_cast<int>(lengthInSeconds % 60);

            juce::String info = file.getFileNameWithoutExtension()
                                + " | "
                                + juce::String(minutes) + ":"
                                + (seconds < 10 ? "0" : "")
                                + juce::String(seconds);

            juce::MessageManager::callAsync([this, info]() {
                fileInfoString = info;
                if (onLoaded) onLoaded(info);
            });
        }
        /*float pitchSemitones = apvts.getRawParameterValue ("PITCH")->load();
        int rootNote = 60 - (int)std::round (pitchSemitones);
        juce::BigInteger range;
        range.setRange(60, 1, true);
        midiPlaybackEngine.addSound(new juce::SamplerSound (
            "Sample", *formatReader, range, rootNote, 0.0, 0.1, 10));*/
        rebuildPadSounds();
    });
}

void SamplerAudioProcessor::rebuildPadSounds()
{
    if (formatReader == nullptr) return;

    midiPlaybackEngine.clearSounds();
    padReaders.clear();

    float pitchSemitones = apvts.getRawParameterValue ("PITCH")->load() + 24;
    int lengthInSamples = int(10 * formatReader->sampleRate);
    for (int i=0; i<NUM_PADS; i++) {
            int midiNote = padMidiNotes[i];
            juce::BigInteger range;
            range.setRange(midiNote, 1, true);
            int startSample = (int)(padTimestamps[i] * formatReader->sampleRate);
            int rootNote = 60 - (int)std::round (pitchSemitones) + i;
            auto reader = std::make_unique<juce::AudioSubsectionReader>(
                formatReader, startSample, lengthInSamples, false);

            midiPlaybackEngine.addSound(new juce::SamplerSound (
                "Sample", *reader, range, rootNote, 0.0, 20.0, 20));

            padReaders.push_back(std::move(reader));
    }
}

/*void SamplerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void SamplerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}*/

juce::AudioProcessorValueTreeState::ParameterLayout SamplerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("CRACKLE", "Crackle", CRACKLE_MIN, CRACKLE_MAX, CRACKLE_DEFAULT));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch", PITCH_MIN, PITCH_MAX, PITCH_DEFAULT));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LOCUT", "LoCut", LOCUT_MIN, LOCUT_MAX, LOCUT_DEFAULT));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HICUT", "HiCut", HICUT_MIN, HICUT_MAX, HICUT_DEFAULT));
    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SamplerAudioProcessor();
}