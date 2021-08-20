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
#include "FFT_Tree_Spectre.h"

class SmartWaveGenerator
{
public:
	SmartWaveGenerator();
	~SmartWaveGenerator();

    void InitOscillators(std::vector< std::vector<OSC_Setting> >& oscs, double deltaTime);
    void InitOscillators(FFT_Tree_Spectre& spectre);
    double* renderWave(int numSamples, int midiNoteNumber);

    void prepareToPlay(int sampleRate, int samplesPerBlock, int outputChanels);

    void ExportOscillatorList(String path);

protected:
    int m_sampleRate { 0 };

private:
    dsp::ProcessSpec spec;
    std::vector<std::unique_ptr<SmartOscillator<double>>> _oscillators;
};

class WaveStorage
{
public:
    WaveStorage(FFT_Tree_Spectre* spectre, BigInteger range);
    ~WaveStorage();
    double* GetWave(int midiNoteNumber, int posititon, int lenght);
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChanels);
    int GetLenght(){ return lenght; }
private:
    void InitWaves(int lenght, BigInteger range);
    std::shared_ptr <SmartWaveGenerator> generator{ nullptr };
    std::map<int, double*> waves;
    FFT_Tree_Spectre* mspectre;
    BigInteger mrange;
    int lenght{ 0 };

};
