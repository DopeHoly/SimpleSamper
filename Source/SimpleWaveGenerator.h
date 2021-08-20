/*
  ==============================================================================

    SimpleWaveGenerator.h
    Created: 22 Apr 2021 7:31:53pm
    Author:  menin

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <map>
#include "OSC_Setting.h"
#include "MyOscillator.h"

class AutoOsc {
public:
    AutoOsc()
    {
        _osc = new MyOscillator<float>{ [](float x) {return sin(x); } };
        _gain = new dsp::Gain<float>();
    }
    ~AutoOsc()
    {
        delete _osc;
        delete _gain;
    }

    MyOscillator<float>* _osc;
    dsp::Gain<float>* _gain;
};

class SimpleWaveGenerator
{
public:
    SimpleWaveGenerator();
    SimpleWaveGenerator(double sampleRate, int samplesPerBlock, int outputChanels);
    ~SimpleWaveGenerator();

    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChanels);

    float* renderWave(int numSamples);
    double* renderWaveDouble(int numSamples);

    void SetFrequency(float freq, int id = 0);
    void SetGain(float gain, int id = 0);
    float GetFrequency(int id = 0);
    float GetGain(int id = 0);
    void SetPhase(float phase, int id = 0);
    float GetPhase(int id = 0);
    void InitOscillators();
    void InitOscillators(std::vector<OSC_Setting>& oscs, int countUsingOsc = 0);

private:

    dsp::ProcessSpec spec;
    //MyOscillator<float> _osc{ [](float x) {return sin(x); } };
    std::vector<std::unique_ptr<AutoOsc>> _oscillators;
    dsp::Gain<float> _gain;
    float gainValue{ 1 };
};

