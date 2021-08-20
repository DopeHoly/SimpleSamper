/*
  ==============================================================================

    SynthVoice.cpp
    Created: 11 Apr 2021 6:07:38pm
    Author:  menin

  ==============================================================================
*/

#include "SynthVoice.h"

SynthSound::SynthSound(
    int rate,
    FFT_Tree_Spectre* spectre,
    const BigInteger& notes,
    uint32 samplePerBlock,
    uint32 numChanels,
    int midiNoteForNormalPitch,
    double attackTimeSecs,
    double releaseTimeSecs)
    :
    SampleRate(rate),
    midiNotes(notes),
    midiRootNote(midiNoteForNormalPitch),
    storage(nullptr)
{
    storage = new WaveStorage(spectre, notes);
    storage->prepareToPlay(rate, samplePerBlock, numChanels);
    length = storage->GetLenght();
    params.attack = static_cast<float> (attackTimeSecs);
    params.release = static_cast<float> (releaseTimeSecs);

    
}

SynthSound::~SynthSound()
{
    delete storage;
}

bool SynthSound::appliesToNote(int midiNoteNumber)
{
    return midiNotes[midiNoteNumber];
}

bool SynthSound::appliesToChannel(int /*midiChannel*/)
{
    return true;
}

double* SynthSound::GetSample(int midiNoteNumber, int position, int numSamples)
{
    return storage->GetWave(midiNoteNumber, position, numSamples);
}




SynthVoice::SynthVoice() {}
SynthVoice::~SynthVoice() {}

bool SynthVoice::canPlaySound(SynthesiserSound* sound)
{
    return dynamic_cast<const SynthSound*> (sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* s, int /*currentPitchWheelPosition*/)
{
    if (auto* sound = dynamic_cast<SynthSound*> (s))
    {
        position = 0;
        midiNote = midiNoteNumber;
        pitchRatio = std::pow(2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
            * sound->SampleRate / getSampleRate();

        lgain = velocity;
        rgain = velocity;
        mVelocity = velocity;
        //osc.setFrequency(440.0f * pitchRatio);
        //sound->setFreq(440.0f * pitchRatio);

        //adsr.setSampleRate(sound->SampleRate);
        //adsr.setParameters(sound->params);

        adsr.noteOn();
    }
    else
    {
        jassertfalse; // this object can only play MySamplerSounds!
    }
}

void SynthVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        clearCurrentNote();
        adsr.reset();
    }
    adsr.noteOff();
}

void SynthVoice::pitchWheelMoved(int /*newValue*/) {}
void SynthVoice::controllerMoved(int /*controllerNumber*/, int /*newValue*/) {}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChanels)
{
    dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChanels;
    mOutputChanels = outputChanels;

    //osc.prepare(spec);
    gain.prepare(spec);
        

    //osc.setFrequency(440.0f);
    gain.setGainLinear(1.0f);

    isPrepared = true;
}

//==============================================================================
void SynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    jassert(isPrepared);
    if (auto* playingSound = static_cast<SynthSound*> (getCurrentlyPlayingSound().get()))
    {
        auto wave = playingSound->GetSample(midiNote, position, numSamples);
        for (int i = 0; i < numSamples; ++i) {
            for (int j = 0; j < mOutputChanels; ++j) {
                outputBuffer.setSample(j, i, wave[i] * mVelocity);
            }
        }

        dsp::AudioBlock<float> audioBlock{ outputBuffer };
        gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        adsr.applyEnvelopeToBuffer(outputBuffer, 0, numSamples);

        position += numSamples;


        if (position > playingSound->length)
        {
            stopNote(0.0f, false);
        }
        
        //auto& osc = playingSound->osc;
        //auto& gain = playingSound->gain;
        /*const float* const inL = data.getReadPointer(0);
        const float* const inR = data.getNumChannels() > 1 ? data.getReadPointer(1) : nullptr;*/

       /* float* outL = outputBuffer.getWritePointer(0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;*/

        //dsp::AudioBlock<float> audioBlock{ outputBuffer };
        //osc.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        //gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        //adsr.applyEnvelopeToBuffer(outputBuffer, 0, numSamples);
        
        //while (--numSamples >= 0)
        //{
        //    //auto pos = (int)sourceSamplePosition;
        //    //auto alpha = (float)(sourceSamplePosition - pos);
        //    //auto invAlpha = 1.0f - alpha;

        //    

        //    // just using a very simple linear interpolation here..
        //    float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        //    float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha)
        //        : l;

        //    auto envelopeValue = adsr.getNextSample();

        //    l *= lgain * envelopeValue;
        //    r *= rgain * envelopeValue;

        //    if (outR != nullptr)
        //    {
        //        *outL++ += l;
        //        *outR++ += r;
        //    }
        //    else
        //    {
        //        *outL++ += (l + r) * 0.5f;
        //    }

        //    sourceSamplePosition += pitchRatio;

        //    if (sourceSamplePosition > playingSound->length)
        //    {
        //        stopNote(0.0f, false);
        //        break;
        //    }
        //}


    }
}