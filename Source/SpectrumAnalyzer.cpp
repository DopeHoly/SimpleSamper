#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer()
{
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::SetSample(AudioSampleBuffer& sample)
{
	mSample = std::make_unique<AudioSampleBuffer>(sample);
}

void SpectrumAnalyzer::SetPosition(int pos)
{
	mPosition = pos;
}

int SpectrumAnalyzer::GetPosition()
{
	return mPosition;
}

std::vector<float> SpectrumAnalyzer::GetFreq()
{
	fftw_cleanup();
	fftw_plan p;
	fftw_complex out[N];
	std::vector<float>  freqVal;
	//подготавливаем данные на вход
	PrepareData();

	//формируем задание для fft
	p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(p);
	fftw_destroy_plan(p);

	for (size_t i = 0; i < N; i++)
	{
		freqVal.push_back(out[i][0]);
	}

	return freqVal;
}

void SpectrumAnalyzer::PrepareData()
{
	auto numSamples = mSample->getNumSamples();
	if (numSamples == 0) return;
	auto buffer = mSample->getReadPointer(0);
	int curPosition;
	for (auto i = 0; i < N; i++) {
		curPosition = mPosition + i;
		in[i][0] = 0;
		if(curPosition < numSamples)
			in[i][0] = buffer[curPosition];
		in[i][1] = 0;
	}
}

