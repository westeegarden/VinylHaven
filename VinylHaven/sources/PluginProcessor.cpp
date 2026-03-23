#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::SamplerSound* SamplerAudioProcessor::loadSound(const juce::String name,
                                  int originalMidiNote,
                                  const std::vector<int>& midiNoteSet,
                                  const void* data,
                                  size_t sizeInBytes) {
    auto inputStream = std::make_unique<juce::MemoryInputStream>(BinaryData::c5_wav, BinaryData::a5_wavSize, false);

    if (auto reader = formatManager.createReaderFor(std::move(inputStream))) {

        juce::BigInteger midiNotes;

        for (auto note : midiNoteSet) {
            midiNotes.setBit(note);
        }

        const double attack = 0.0;
        const double release = 0.1;
        const double sampleLength = 10.0;

        // C5 sound
        return new juce::SamplerSound(name, *reader, midiNotes, originalMidiNote, attack, release, sampleLength);
    }
}

SamplerAudioProcessor::SamplerAudioProcessor() :
    AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();

    // Only 1 voice, so only 1 sample plays at a given moment
    midiPlaybackEngine.addVoice(new juce::SamplerVoice);

    midiPlaybackEngine.addSound(loadSound("C5", 60, { 36 }, BinaryData::c5_wav, BinaryData::c5_wavSize));
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
    midiPlaybackEngine.setCurrentPlaybackSampleRate(newSampleRate);
}

void SamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    clearUnusedOutputChannels(buffer);
    midiPlaybackEngine.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

void SamplerAudioProcessor::clearUnusedOutputChannels(juce::AudioBuffer<float>& buffer) const
{
    for ( auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
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

void SamplerAudioProcessor::loadFile() {
    chooser = std::make_unique<juce::FileChooser>("Please load a file");

    auto flags = juce::FileBrowserComponent::openMode |
                 juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file.existsAsFile())
        {
            formatReader = formatManager.createReaderFor(file);
        }
        juce::BigInteger range;
        range.setRange(0, 128, true);
        midiPlaybackEngine.addSound(new juce::SamplerSound (
            "Sample", *formatReader, range,60, 0.1, 0.1, 10));
    });
}

juce::AudioProcessorValueTreeState::ParameterLayout SamplerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SamplerAudioProcessor();
}