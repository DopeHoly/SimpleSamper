/*
  ==============================================================================

    SynthVoice.h
    Created: 11 Apr 2021 6:07:38pm
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SmartWaveGenerator.h"
#include "FFT_Tree_Spectre.h"

class SynthSound : public SynthesiserSound
{
public:
    SynthSound(
        int rate,
        FFT_Tree_Spectre* spectre,
        const BigInteger& midiNotes,
        uint32 samplePerBlock,
        uint32 numChanels,
        int midiNoteForNormalPitch,
        double attackTimeSecs,
        double releaseTimeSecs);

    /** Destructor. */
    ~SynthSound() override;

    //==============================================================================
    /** Changes the parameters of the ADSR envelope which will be applied to the sample. */
    void setEnvelopeParameters(ADSR::Parameters parametersToUse) { params = parametersToUse; }

    //==============================================================================
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int midiChannel) override;

    double* GetSample(int midiNoteNumber, int position, int numSamples);

private:
    friend class SynthVoice;
    //==============================================================================

    WaveStorage* storage;
    int SampleRate;
    BigInteger midiNotes;
    int length{ 0 };
    int midiRootNote{ 0 };
    ADSR::Parameters params;


    JUCE_LEAK_DETECTOR(SynthSound)
};

class SynthVoice : public SynthesiserVoice
{
public:
    SynthVoice();
    ~SynthVoice() override;
    //==============================================================================
    bool canPlaySound(SynthesiserSound*) override;

    void startNote(int midiNoteNumber, float velocity, SynthesiserSound*, int pitchWheel) override;
    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChanels);

    void renderNextBlock(AudioBuffer<float>&, int startSample, int numSamples) override;
    using SynthesiserVoice::renderNextBlock;

    ADSR& getADSR() { return adsr; }
private:
    //==============================================================================
    double pitchRatio = 0;
    float lgain = 0, rgain = 0;
    int position{ 0 };
    int midiNote{ 0 };
    int mOutputChanels{ 0 };
    double mVelocity{ 0 };

    //juce::dsp::Oscillator<float> osc{ [](float x) {return sin(x); } };
    dsp::Gain<float> gain;
    ADSR adsr;
    bool isPrepared{ false };

    JUCE_LEAK_DETECTOR(SynthVoice)
};