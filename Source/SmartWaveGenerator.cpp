/*
  ==============================================================================

    SmartWaveGenerator.cpp
    Created: 6 May 2021 8:56:10pm
    Author:  menin

  ==============================================================================
*/

#include "SmartWaveGenerator.h"
#include "ValuesFunc.h"

SmartWaveGenerator::SmartWaveGenerator(double sampleRate)
{
    m_sampleRate = sampleRate;

    spec.maximumBlockSize = 50000;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
}

SmartWaveGenerator::~SmartWaveGenerator()
{
    _oscillators.shrink_to_fit();
}

int FindOSC_settings(std::vector<OSC_Setting>& frame, double value, double treshold) {

    std::vector<double> difs;
    for (size_t i = 0; i < frame.size(); i++)
    {
        auto item = frame.at(i);
        auto dif = abs(item.freq - value);
        difs.push_back(dif);
    }
    double minValue = INFINITY;
    size_t minId = -1;
    for (size_t i = 0; i < difs.size(); i++) {
        auto item = difs.at(i);
        if (item < minValue && item < treshold) {
            minValue = item;
            minId = i;
        }
    }
    return minId;
}


void SmartWaveGenerator::InitOscillators(std::vector< std::vector<OSC_Setting> >& oscs, int stepDelta)
{
    const double treshold = 1.0 / 24.0;
    const double deltaTime = ((double)stepDelta) / ((double)m_sampleRate);

    double currenttime;

    double currentNewMarkerID = 0;

    std::vector<OSC_Setting> prevFrame = oscs.at(0);
    for each (auto var in prevFrame)
    {
        var.marker = currentNewMarkerID;
        ++currentNewMarkerID;
    }

    for (auto i = 1; i < oscs.size(); ++i)
    {
        currenttime = (i + 1) * deltaTime;

        auto curentFrame = oscs.at(i);
        for each (auto item in curentFrame)
        {
            auto id = FindOSC_settings(prevFrame, item.freq, treshold);
            if (id == -1) {
                item.marker = currentNewMarkerID;
                ++currentNewMarkerID;

            }
            else {
                item.marker = prevFrame.at(id).marker;
            }
        }
    }


    std::vector<ValuesFunc> freqs(currentNewMarkerID);
    std::vector<ValuesFunc> gains(currentNewMarkerID);
    std::vector<double> phase(currentNewMarkerID);

    int curID = -1;
    for (auto i = 1; i < oscs.size(); ++i)
    {
        currenttime = (i + 1) * deltaTime;

        auto curentFrame = oscs.at(i);
        for each (auto item in curentFrame) {
            if (item.marker > curID) {
                curID = item.marker;
                freqs.push_back(ValuesFunc());
                gains.push_back(ValuesFunc());
                phase.push_back(item.phase);
            }
            if (i == 0) {
                freqs.at(item.marker).AddPoint(0, item.freq);
                gains.at(item.marker).AddPoint(0, item.vol);
            }
            freqs.at(item.marker).AddPoint(currenttime, item.freq);
            gains.at(item.marker).AddPoint(currenttime, item.vol);
        }
    }

    for (int k = 0; k < currentNewMarkerID; ++k) {
        auto autoOsc = std::make_unique <SmartOscillator<double>>();
        autoOsc->prepare(spec, freqs.at(k), gains.at(k));

        autoOsc->SetPhase(phase.at(k));

        _oscillators.push_back(std::move(autoOsc));
    }
}

float* SmartWaveGenerator::renderWave(int numSamples)
{
    auto buffer = new float[numSamples];

    for (int i = 0; i < numSamples; ++i)
        buffer[i] = 0.0;

    for (int j = 0; j < _oscillators.size(); ++j) {

        _oscillators[j]->reset();
    }

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < _oscillators.size(); ++j) {

            buffer[i] += _oscillators[j]->processSample();
        }
    }

    return buffer;
}