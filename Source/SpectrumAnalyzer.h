#pragma once
#define _USE_MATH_DEFINES

#include <JuceHeader.h>
#include <math.h>
#include <fftw3.h>
#define N 2048

class SpectrumAnalyzer
{
public:
	SpectrumAnalyzer();
	~SpectrumAnalyzer();

	void SetSample(AudioSampleBuffer& sample);
	void SetPosition(int pos);
	int GetPosition();
	std::vector<float>  GetFreq();

private:
	int mPosition{ 0 };
	std::unique_ptr<AudioSampleBuffer> mSample;
	fftw_complex in[N];
	void PrepareData();

};