#pragma once

#include "FFTNode.h"
#include "FFT_Tree_Spectre.h"
#include <JuceHeader.h>
#include <vector>


class Controller {
public:
	Controller(int bottomPow, int topPow, int sampleRate = 44100);
	~Controller();
	void ClearData();
	void SetSample(AudioSampleBuffer& sample);
	std::vector<std::shared_ptr<FFTNode>>* GetLeafs();
	AudioSampleBuffer& GetSample();
	int GetNumSamples();
	int GetBottomPow();
	int GetTopPow();
	bool SampleLoaded();
	int GetSampleRate();
	void SetSampleRate(int value);
	FFT_Tree_Spectre* GetSpectre();

private:
	std::vector<std::shared_ptr<FFTNode>> elements;
	std::shared_ptr<AudioSampleBuffer> _sample;
	int mSampleRate;
	int TopPow;
	int BottomPow;
	bool _sampleLoaded{ false };

	void InitElements();
};