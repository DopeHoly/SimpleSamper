/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <omp.h>

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
                       ), 
    mAPVTS(*this, nullptr, "PARAMETERS", createParameters())
#endif
{

    omp_set_num_threads(12);
    mAPVTS.state.addListener(this);
    mFormatManager.registerBasicFormats(); 
    //transportSource.addChangeListener(this);
    recreateSamplerVoices(16);
}

SimpleSamperAudioProcessor::~SimpleSamperAudioProcessor()
{
    mFormatReader = nullptr;
    transportSource.releaseResources();
    mSampler.clearSounds();
    mSampler.clearVoices();
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
    mSpectrumAnalyzer.prepareToPlay(sampleRate, samplesPerBlock);
    SampleRate = sampleRate;
    mSamplesPerBlock = samplesPerBlock;
    updateADSR();
    auto numVoice = mSampler.getNumVoices();
    for (int i = 0; i < numVoice; ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(mSampler.getVoice(i))) {
            voice->prepareToPlay(SampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
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
    if (isPlayng) {
        AudioSourceChannelInfo bufferToFill(buffer);
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock(bufferToFill);
        isPlayng = transportSource.isPlaying();
        return;
    }

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateADSR();
    //mSampler.getVoice(0).
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

    for (int i = 0; i < numVoices; ++i) {
        //mSampler.addVoice(new MySamplerVoice());
        mSampler.addVoice(new SynthVoice());
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

    if (mFormatReader != nullptr) {
        auto sampleLength = static_cast<int>(mFormatReader->lengthInSamples);

        if (sampleLength > (mFormatReader->sampleRate * 20)) {
            NativeMessageBox::showMessageBox(
                AlertWindow::AlertIconType::WarningIcon,
                "Warning", 
                "Audio file longer than 20 seconds.");
            return;

        }

        mWaveForm.clear();
        mSpectrumAnalyzer.Clear();

        mWaveForm.setSize(1, sampleLength);
        mFormatReader->read(&mWaveForm, 0, sampleLength, 0, true, false);

        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(mFormatReader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, mFormatReader->sampleRate);
        readerSource.reset(newSource.release());

        FileLoaded.sendActionMessage("");
        //StartCalculation();
    }
    else {
        NativeMessageBox::showMessageBox(
            AlertWindow::AlertIconType::WarningIcon,
            "Warning",
            "Can't read audio file.");
    }
}

void SimpleSamperAudioProcessor::updateADSR() 
{
    if (!mShouldUpdate) return;
    mADSRparameters.attack = mAPVTS.getRawParameterValue("ATTACK")->load();
    mADSRparameters.decay = mAPVTS.getRawParameterValue("DECAY")->load();
    mADSRparameters.sustain = mAPVTS.getRawParameterValue("SUSTAIN")->load();
    mADSRparameters.release = mAPVTS.getRawParameterValue("RELEASE")->load();

    for (int i = 0; i < mSampler.getNumSounds(); ++i)
    {
        if (auto sound = dynamic_cast<MySamplerSound*>(mSampler.getSound(i).get()))
        {
            sound->setEnvelopeParameters(mADSRparameters);
        }
    }
    mShouldUpdate = false;
}

void SimpleSamperAudioProcessor::StartCalculation()
{
    mSampler.clearSounds();

    mSpectrumAnalyzer.SetSample(mWaveForm);
    BigInteger range;
    range.setRange(0, 128, true);
    spectre = mSpectrumAnalyzer.GetFFT_Tree();

    StartCalculationLoaded.sendActionMessage("");

    auto sound = new SynthSound(
        getSampleRate(), 
        spectre, 
        range, 
        mSamplesPerBlock, 
        getTotalNumOutputChannels(), 
        60, 
        0.1,
        0.1
    );

    sound->setEnvelopeParameters(mADSRparameters);

    mSampler.addSound(sound);

}

void SimpleSamperAudioProcessor::PlaySample()
{
    if (transportSource.isPlaying()) {
        transportSource.stop();
    }
    else {
        transportSource.setPosition(0.0);
        transportSource.start();
        isPlayng = true;
    }
}

AudioProcessorValueTreeState::ParameterLayout SimpleSamperAudioProcessor::createParameters() 
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<AudioParameterFloat>("ATTACK", "Attack", 0.0f, 5.0f, 0.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("DECAY", "Decay", 0.0f, 3.0f, 2.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 5.0f, 0.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("RELEASE", "Release", 0.0f, 5.0f, 0.0f));
    return { parameters.begin(), parameters.end() };
}

void SimpleSamperAudioProcessor::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
    mShouldUpdate = true;
    updateADSR();
}

