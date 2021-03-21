#pragma once

#include <JuceHeader.h>
//#include <juce_audio_formats/sampler/juce_Sampler.h>

class MySamplerSound : public SynthesiserSound
{
public:
    MySamplerSound(const String& name,
        AudioFormatReader& source,
        const BigInteger& midiNotes,
        int midiNoteForNormalPitch,
        double attackTimeSecs,
        double releaseTimeSecs,
        double maxSampleLengthSeconds);
    /** Destructor. */
    ~MySamplerSound() override;

    /** Returns the sample's name */
    const String& getName() const noexcept { return name; }

    /** Returns the audio sample data.
        This could return nullptr if there was a problem loading the data.
    */
    AudioBuffer<float>* getAudioData() const noexcept { return data.get(); }

    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    void setEnvelopeParameters(ADSR::Parameters parametersToUse) { params = parametersToUse; }

    //==============================================================================
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;

private:
    //==============================================================================
    friend class MySamplerVoice;

    String name;
    std::unique_ptr<AudioBuffer<float>> data;
    double sourceSampleRate;
    BigInteger midiNotes;
    int length = 0, midiRootNote = 0;

    ADSR::Parameters params;

    JUCE_LEAK_DETECTOR(SamplerSound)
};

class MySamplerVoice : public SynthesiserVoice
{
public:
    MySamplerVoice();
    ~MySamplerVoice() override;
    //==============================================================================
    bool canPlaySound(SynthesiserSound*) override;

    void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void renderNextBlock(AudioBuffer<float>&, int startSample, int numSamples) override;
    using SynthesiserVoice::renderNextBlock;

    double getSamplePosition() { return sourceSamplePosition; }
    ADSR& getADSR() { return adsr; }
private:
    //==============================================================================
    double pitchRatio = 0;
    double sourceSamplePosition = 0;
    float lgain = 0, rgain = 0;

    ADSR adsr;
    //std::shared_ptr<ADSR> adsr;

    JUCE_LEAK_DETECTOR(SamplerVoice)
};

