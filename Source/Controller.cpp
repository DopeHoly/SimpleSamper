#pragma once

#include "Controller.h"

Controller::Controller(int bottomPow, int topPow, int sampleRate) :
	BottomPow(bottomPow),
	TopPow(topPow),
	_sample(nullptr),
	mSampleRate(sampleRate),
	_sampleLoaded(false)
{
}

Controller::~Controller()
{

}

void Controller::ClearData()
{
	_sampleLoaded = false;
	elements.clear();
}


void Controller::SetSample(AudioSampleBuffer& sample)
{
	_sample = std::make_shared<AudioSampleBuffer>(sample);
	_sampleLoaded = true;
	InitElements();
}

AudioSampleBuffer& Controller::GetSample()
{
	return *_sample;
}

int Controller::GetNumSamples()
{
	if (&_sample == nullptr)
		throw new Expression(1);

	auto numSamples = _sample->getNumSamples();
	return numSamples;
}

int Controller::GetBottomPow()
{
	return BottomPow;
}

int Controller::GetTopPow()
{
	return TopPow;
}

bool Controller::SampleLoaded()
{
	return _sampleLoaded;
}

int Controller::GetSampleRate()
{
	return mSampleRate;
}

void Controller::SetSampleRate(int value)
{
	mSampleRate = value;
}

std::vector<std::shared_ptr<FFTNode>>* Controller::GetLeafs()
{
	auto result = new std::vector<std::shared_ptr<FFTNode>>();
	for each (auto item in elements)
	{
		item->AddLeaf(result);
	}
	return result;
}

void Controller::InitElements()
{
	int windowsSize = 1 << TopPow;
	if (&_sample == nullptr)
		throw new Expression(1);

	auto numSamples = _sample->getNumSamples();

	FFTNode* parent = nullptr;

	int position = 0;
	int nextPosition;
	int cnt = 0;
	while (position < numSamples) {
		nextPosition = position + windowsSize;
		//auto node = new FFTNode();
		elements.push_back(std::make_shared<FFTNode>(this, position, TopPow, nullptr, String(cnt)));
		position = nextPosition;
		++cnt;
	}

}

FFT_Tree_Spectre* Controller::GetSpectre()
{
	auto leafs = GetLeafs();
	auto ZeroBinSpectre = leafs->at(0)->GetSumFunc();
	auto numFreq = ZeroBinSpectre.GetSize();
	auto numBin = leafs->size();
	auto length = 1 << BottomPow;
	auto timeToBin = double(length) / double(mSampleRate);

	auto spectre = new FFT_Tree_Spectre(numBin, elements.size(), numFreq, timeToBin);
	spectre->difPow = TopPow - BottomPow;

	//spectre->numBins = 1 << BottomPow;

	auto sumFreqZero = leafs->at(0)->GetSumFunc();
	for (int i = 0; i < sumFreqZero.GetSize(); ++i) {
		spectre->frequancy[i] = sumFreqZero.GetValueAt(i).x;
	}


	int counter = 0;
	for each (auto item in elements)
	{
		auto phases = item->GetPhases();
		for (int i = 0; i < numFreq; ++i) {
			auto val = phases.GetValueAt(i);
			spectre->SetPhase(counter, i, val.y);
		}
		++counter;
	}

	int cnt = 0;
	for each (auto item in *leafs)
	{
		auto sumFreq = (item->GetSumFunc()) * double(1.0 / spectre->difPow);
		for (int i = 0; i < sumFreq->GetSize(); ++i) {
			spectre->SetValue(cnt, i, sumFreq->GetValueAt(i).y);
		}
		free(sumFreq);
		++cnt;
	}
	//leafs->clear();
	return spectre;
}