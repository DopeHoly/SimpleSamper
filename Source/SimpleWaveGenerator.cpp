/*
  ==============================================================================

    SimpleWaveGenerator.cpp
    Created: 22 Apr 2021 7:31:53pm
    Author:  menin

  ==============================================================================
*/

#include "SimpleWaveGenerator.h"

SimpleWaveGenerator::SimpleWaveGenerator(double sampleRate, int samplesPerBlock, int outputChanels)
{
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;

    //_osc.prepare(spec);
    //_gain.prepare(spec);

    //_osc.setFrequency(440.0f);
    //_gain.setGainLinear(0.5f);
}

SimpleWaveGenerator::~SimpleWaveGenerator()
{
    _oscillators.clear();
}

float* SimpleWaveGenerator::renderWave(int numSamples)
{
    auto buffer = new float[numSamples];

    for (int i = 0; i < numSamples; ++i)
        buffer[i] = 0.0;

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < _oscillators.size(); ++j) {

            buffer[i] += _oscillators[j]->_gain->processSample(_oscillators[j]->_osc->processSample(0.0f));
        }
    }

    return buffer;
}

void SimpleWaveGenerator::SetFrequency(float freq, int id)
{
    _oscillators[id]->_osc->setFrequency(freq);
}

void SimpleWaveGenerator::SetGain(float gain, int id)
{
    //auto dbGain = 0;//20 * log10(gain);
    //gainValue = 20 * log10(gain / 43.353780883961484);
    //gainValue = 1/(20 * log10(gain));
    //_gain.setGainLinear(gain/42.0f);
    _oscillators[id]->_gain->setGainDecibels(gain);
}

float SimpleWaveGenerator::GetFrequency(int id)
{
    return _oscillators[id]->_osc->getFrequency();
}

float SimpleWaveGenerator::GetGain(int id)
{
    return _oscillators[id]->_gain->getGainDecibels();
}

void SimpleWaveGenerator::SetPhase(float phase, int id)
{
    _oscillators[id]->_osc->SetPhase(phase);
}

float SimpleWaveGenerator::GetPhase(int id)
{
    return _oscillators[id]->_osc->GetPhase();
}

void SimpleWaveGenerator::InitOscillators()
{
    _oscillators.clear();
    auto autoOsc = std::make_unique <AutoOsc>();

    autoOsc->_osc->prepare(spec);
    autoOsc->_gain->prepare(spec);

    autoOsc->_osc->setFrequency(440);
    autoOsc->_osc->SetPhase(0);
    autoOsc->_gain->setGainLinear(0.5);

    _oscillators.push_back(std::move(autoOsc));
}

void SimpleWaveGenerator::InitOscillators(std::vector<OSC_Setting>& oscs, int countUsingOsc)
{
    _oscillators.clear();
    if (countUsingOsc == 0)
        countUsingOsc = oscs.size();
    int counter = 0;
    for each (auto item in oscs)
    {
        if (counter == countUsingOsc) break;
        auto autoOsc = std::make_unique <AutoOsc>();

        autoOsc->_osc->prepare(spec);
        autoOsc->_gain->prepare(spec);

        autoOsc->_osc->setFrequency(item.freq);
        autoOsc->_osc->SetPhase(item.phase);
        autoOsc->_gain->setGainDecibels(item.vol + 88.6);

        _oscillators.push_back(std::move(autoOsc));
        ++counter;
    }
}
