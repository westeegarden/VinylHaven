#include "PluginProcessor.h"
#include "PluginEditor.h"


SamplerAudioProcessor::SamplerAudioProcessor() :
    AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();

    // Only 1 voice, so only 1 sample plays at a given moment
    midiPlaybackEngine.addVoice(new juce::SamplerVoice);
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
        juce::BigInteger range;
        range.setRange(0, 128, true);
        midiPlaybackEngine.addSound(new juce::SamplerSound (
            "Sample", *formatReader, range,60, 0.0, 0.1, 10));
    });
}

juce::AudioProcessorValueTreeState::ParameterLayout SamplerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch", PITCH_MIN, PITCH_MAX, PITCH_DEFAULT));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LOCUT", "LoCut", LOCUT_MIN, LOCUT_MAX, LOCUT_DEFAULT));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HICUT", "HiCut", HICUT_MIN, HICUT_MAX, HICUT_DEFAULT));
    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SamplerAudioProcessor();
}