/*
  ==============================================================================

    SmartWaveGenerator.h
    Created: 6 May 2021 8:56:10pm
    Author:  menin

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OSC_Setting.h"
#include "SmartOscillator.h"


class SmartWaveGenerator
{
public:
	SmartWaveGenerator(double sampleRate);
	~SmartWaveGenerator();

    void InitOscillators(std::vector< std::vector<OSC_Setting> >& oscs, int stepDelta);
    float* renderWave(int numSamples);

private:
    int m_sampleRate;
    dsp::ProcessSpec spec;
    std::vector<std::unique_ptr<SmartOscillator<double>>> _oscillators;
};
