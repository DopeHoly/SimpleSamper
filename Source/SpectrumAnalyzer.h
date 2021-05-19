#pragma once
#define _USE_MATH_DEFINES

#include <JuceHeader.h>
#include <math.h>
#include <fftw3.h>
#include "SimpleWaveGenerator.h"
#include "OSC_Setting.h"
//#define N 32768
//#define N 8192
//#define N 2048

typedef struct
{
	double r;
	double i;
} mycomplex;

enum FFTMethod {
	Juce,
	FFTW,
	Mine,
	Overlap
};



class SpectrumAnalyzer
{
public:
	SpectrumAnalyzer(double sampleRate, int samplesPerBlock);
	~SpectrumAnalyzer();

	void SetSample(AudioSampleBuffer& sample);
	void SetPosition(int pos);
	int GetPosition();
	std::vector<float>  GetFreq();
	std::unique_ptr<std::vector<OSC_Setting>> FFTSpectre();
	int getWindowSize() { return mWindowSize; }
private:
	int mSampleRate;
	int mPosition{ 0 };
	int winPow{ 15 };
	int mWindowSize{ 2 << winPow };
	bool CorrectPhaseActive{ false };
	bool UseWindow{ false };
	std::unique_ptr<AudioSampleBuffer> mSample;

	SimpleWaveGenerator mWaveGenerator;
	//fftw_complex in[N];

	void CorrectPhase(std::vector<OSC_Setting>& data);

	fftw_complex* PrepareData(int position);
	fftw_complex* PrepareData(float* buffer, int windowsSize, int position);
	juce::dsp::Complex<float>* PrepareDataJuceComplex(float* buffer, int windowsSize, int position);
	float* GetSourceWave(int position, int windowsSize);

	mycomplex* PreapareComplex(int position);
	mycomplex* PreapareComplex(float* buffer, int windowsSize, int position);

	double windowHanning(double n, double frameSize);
	double Gausse(double n, double frameSize);
	double windowsSum(double frameSize);

	double Align(double angle, double period);

	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrum(fftw_complex* spec0, fftw_complex* spec1, double shiftsPerFrame);
	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrum(mycomplex* spec0, mycomplex* spec1, double shiftsPerFrame);

	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrumSimple(fftw_complex* spec0);
	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrumSimple(juce::dsp::Complex<float>* spec0);
	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrumSimple(mycomplex* spec0);

	
	bool ExportFrequancyToCSV(String path, std::vector<OSC_Setting>& dict);
	bool ExportWaveToCSV(String path, float* data, int size, int startPos = 0);
	bool ExportComplexToCSV(String path, fftw_complex* data, int size);
	bool ExportComplexToCSV(String path, mycomplex* data, int size);
	bool ExportComplexToCSV(String path, juce::dsp::Complex<float>* data, int size);

	fftw_complex* PerformFFT(float* wave, int size, int position = 0);
	mycomplex* PerformFFT2(float* wave, int size, int position = 0);
	juce::dsp::Complex<float>* PerformFFTJuce(float* wave, int size, int position = 0);


	std::unique_ptr<std::vector<OSC_Setting>> GetJoinedSpectrumWave(float* wave, int size, int step);

	OSC_Setting& findMaxFreq(std::vector<OSC_Setting>& dict);
	int findMaxFreqId(std::vector<OSC_Setting>& dict);
	void CalcPhaseDif(); 
	double GetCenterMassXMaxFreq(std::vector<OSC_Setting>& data);

	std::unique_ptr<std::vector<OSC_Setting>> Antialiasing(std::vector<OSC_Setting>& dict);

	void fftr(mycomplex a[], int n);
};

