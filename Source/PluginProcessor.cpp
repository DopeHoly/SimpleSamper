/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSamperAudioProcessor::SimpleSamperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mFormatManager.registerBasicFormats();
    recreateSamplerVoices();
}

SimpleSamperAudioProcessor::~SimpleSamperAudioProcessor()
{
    mFormatReader = nullptr;
}

//==============================================================================
const juce::String SimpleSamperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSamperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSamperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSamperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleSamperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleSamperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleSamperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleSamperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleSamperAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleSamperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleSamperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void SimpleSamperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleSamperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleSamperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool SimpleSamperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleSamperAudioProcessor::createEditor()
{
    return new SimpleSamperAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleSamperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleSamperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSamperAudioProcessor();
}

void SimpleSamperAudioProcessor::recreateSamplerVoices(int numVoices) 
{
    mSampler.clearVoices();
    if (numVoices == 0) {
        numVoices = mNumVoices;
    }
    //создание голосов сэмплера
    for (int i = 0; i < numVoices; ++i) {
        mSampler.addVoice(new SamplerVoice());
    }
}

void SimpleSamperAudioProcessor::loadFileFromOpenFileDialog() 
{
    mSampler.clearSounds();
    FileChooser chooser{ "Pleace load a file" };

    if (chooser.browseForFileToOpen()) {
        auto file = chooser.getResult();
        loadFile(file.getFullPathName());
    }
}


void SimpleSamperAudioProcessor::loadFile(const String& path)
{
    mSampler.clearSounds();

    auto file = File(path);
    mFormatReader = mFormatManager.createReaderFor(file);
    BigInteger range;
    range.setRange(0, 128, true);//?

    mSampler.addSound(new SamplerSound("Sample", *mFormatReader, range, 60, 0.1, 0.1, 10.0));
}

