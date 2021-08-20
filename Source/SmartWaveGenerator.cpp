/*
  ==============================================================================

    SmartWaveGenerator.cpp
    Created: 6 May 2021 8:56:10pm
    Author:  menin

  ==============================================================================
*/

#include "SmartWaveGenerator.h"
#include <algorithm>
#include "ValuesFunc.h"
#include "PeakFinder.h"
#include "MyPeakFinder.h"
#include <fstream>
#include <string>
#include "PeakFinder.cpp"

SmartWaveGenerator::SmartWaveGenerator():
    m_sampleRate(1),
    spec()
{
    m_sampleRate = 0;
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


void SmartWaveGenerator::InitOscillators(std::vector< std::vector<OSC_Setting> >& oscs, double deltaTime)
{
    const double treshold = 3.0;
    //const double deltaTime = stepDelta;

    double currenttime;

    double currentNewMarkerID = 0;

    std::vector<OSC_Setting>& prevFrame = oscs.at(0);
    //for each (OSC_Setting& var in prevFrame)
    //{
    //    var.marker = currentNewMarkerID;
    //    ++currentNewMarkerID;
    //}

    for (auto i = 0; i < prevFrame.size(); ++i)
    {
        prevFrame.at(i).marker = currentNewMarkerID;
        ++currentNewMarkerID;
    }

    for (auto i = 1; i < oscs.size(); ++i)
    {
        std::vector<OSC_Setting>& curentFrame = oscs.at(i);
        for (auto j = 0; j < curentFrame.size(); ++j)
        //for each (auto item in curentFrame)
        {
            OSC_Setting& item = curentFrame.at(j);
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


    std::vector<ValuesFunc> freqs;
    std::vector<ValuesFunc> gains;
    std::vector<double> phase;

    int curID = -1;
    for (auto i = 0; i < oscs.size(); ++i)
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
            if (i == 0) {//TODO
                freqs.at(item.marker).AddPoint(0, item.freq);
                gains.at(item.marker).AddPoint(0, 0);
            }
            freqs.at(item.marker).AddPoint(currenttime, item.freq);
            gains.at(item.marker).AddPoint(currenttime, item.vol);
        }
    }

    for (int k = 0; k < currentNewMarkerID; ++k) {
        auto autoOsc = std::make_unique <SmartOscillator<double>>([](float x) {return sin(x); });
        if (freqs.at(k).GetSize() < 10 ||
            gains.at(k).GetSize() < 10 
            ) continue;
        autoOsc->prepare(spec, freqs.at(k), gains.at(k), phase.at(k));

        _oscillators.push_back(std::move(autoOsc));
    }
    //ExportOscillatorList("D:\\YandexDisk\\oscillatorsSaw.csv");
    //for (auto it = _oscillators.begin(); it != _oscillators.end(); ) {
    //    if ((*it)-> % 2 == 0) {
    //        it = c.erase(it);
    //    }
    //    else {
    //        ++it;
    //    }
    //}
    //фильтрация осцилляторов

}

std::vector< OSC_Setting> GetPeaks(FFT_Tree_Spectre& spectre, int bin) 
{
    std::vector<float> volume;
    std::vector<int> out;
    std::vector<OSC_Setting> peaks;

    for (int i = 0; i < spectre.numFrequeance; ++i) 
    {
        volume.push_back(spectre.GetValue(bin, i));
    }
    //PeakFinder::findPeaks(volume, out);
    myfindPeaks(volume, out);
    for each (auto id in out)
    {
        peaks.push_back(OSC_Setting(
            spectre.frequancy[id],
            spectre.GetValue(bin, id),
            spectre.GetPhase(bin, id)
        ));
    }

    return peaks;
}

void SmartWaveGenerator::InitOscillators(FFT_Tree_Spectre& spectre)
{
    const double treshold = 1.0 / 24.0;
    const double deltaTime = spectre.timeToBin;

    double currenttime;

    double currentNewMarkerID = 0;

    auto peaks = std::vector< std::vector<OSC_Setting> >();

    //Выдёргивание из спектрограммы пики по каждому фрейму
    for (int i = 0; i < spectre.numBins; ++i) 
    {
        auto binPeaks = GetPeaks(spectre, i);
        peaks.push_back(binPeaks);
    }
    InitOscillators(peaks, deltaTime);
}

double* SmartWaveGenerator::renderWave(int numSamples, int midiNoteNumber)
{
    auto buffer = new double[numSamples];

    for (int i = 0; i < numSamples; ++i)
        buffer[i] = 0.0;

    for (int j = 0; j < _oscillators.size(); ++j) {

        _oscillators[j]->reset();
    }

    for (int i = 0; i < numSamples; ++i) {
        for (int j = 0; j < _oscillators.size(); ++j) {

            buffer[i] += _oscillators[j]->processSample(midiNoteNumber);
        }
    }

    return buffer;
}

void SmartWaveGenerator::prepareToPlay(int sampleRate, int samplesPerBlock, int outputChanels)
{
    m_sampleRate = sampleRate;

    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChanels;
}

void PrintValueFunc(std::ofstream& myfile, ValuesFunc& func) {
    auto size = func.GetSize();
    for (size_t i = 0; i < size; i++)
    {
        auto valueX = std::to_string(func.GetValueAt(i).x);
        //auto valueY = std::to_string(func.GetValueAt(i).y);
        std::replace(valueX.begin(), valueX.end(), '.', ',');
        myfile << valueX << ";";
    }
    myfile << std::endl;    
    for (size_t i = 0; i < size; i++)
    {
        auto valueY = std::to_string(func.GetValueAt(i).y);
        std::replace(valueY.begin(), valueY.end(), '.', ',');
        myfile << valueY << ";";
    }
    myfile << std::endl;
}

void SmartWaveGenerator::ExportOscillatorList(String path)
{
    auto file = File(path);
    std::ofstream myfile;
    myfile.open(path.toStdString());
    auto cnt = _oscillators.size();
    for (int i = 0; i < cnt; ++i) {
        auto item = _oscillators.at(i).get();
        auto freq = item->GetFrequency();
        auto gain = item->GetGain();
        auto phase = item->GetPhase();
        myfile << "Oscillator" << i << std::endl;
        myfile << "freq" << std::endl;
        PrintValueFunc(myfile, freq);
        myfile << "gain" << std::endl;
        PrintValueFunc(myfile, gain);
        myfile << "phase;" << phase << std::endl << std::endl;
    }
    myfile.close();
}


WaveStorage::WaveStorage(FFT_Tree_Spectre* spectre, BigInteger range) :
    generator(),
    mspectre(spectre),
    mrange(range),
    waves()
{
    generator = std::make_shared <SmartWaveGenerator>();
}

WaveStorage::~WaveStorage()
{
    waves.clear();
}

double* WaveStorage::GetWave(int midiNoteNumber, int position, int lenght)
{
    auto wave = (waves.find(midiNoteNumber)->second);
    auto resultWave = new double[lenght];
    auto len = this->lenght;
    int pos;  
//#pragma omp parallel for
    for (int i = 0; i < lenght; ++i) {
        pos = i + position;
        if (pos >= len) {
            resultWave[i] = 0.0;
        }
        else {
            resultWave[i] = wave[pos];
        }
    }
    //std::copy(wave + position, wave + position + lenght, resultWave);
    return resultWave;
}

void WaveStorage::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChanels)
{
    generator->prepareToPlay(sampleRate, samplesPerBlock, outputChanels);
    generator->InitOscillators(*mspectre);
    lenght = trunc(mspectre->numBins * mspectre->timeToBin * sampleRate);
    InitWaves(lenght, mrange);
}

void WaveStorage::InitWaves(int lenght, BigInteger range)
{
//#pragma omp parallel for
    for (int i = 36; i <= 84; ++i) {
        waves.insert(std::make_pair(i, generator->renderWave(lenght, i)));
    }
}
