#pragma once

#include "FFTNode.h"
#include "Controller.h"


FFTNode::FFTNode(Controller* control, int begin, int pow, FFTNode* par, String index) :
	controller(control),
	parent(par),
	windowPow(pow),
	_begin(begin),
	func(nullptr),
	funcPhase(nullptr),
	sumFunc(nullptr),
	_index(index)//,
	//complexValue(nullptr)
{
	func = std::shared_ptr<ValuesFunc>(new ValuesFunc());

	if (controller->GetTopPow() == pow)
		funcPhase = std::shared_ptr<ValuesFunc>(new ValuesFunc());

	windowSize = 1 << pow;
	//_end = _begin + windowSize;

	outOfRange = controller->GetNumSamples() <= begin;

	if(outOfRange) return;

	Recalc();

	if (controller->GetBottomPow() == pow) return;
	
	InitChilds();
}

FFTNode::~FFTNode()
{
	//delete func;
	//delete funcPhase;
	//delete sumFunc;
	//delete complexValue;
	
	//if (controller->GetBottomPow() == windowPow || outOfRange) return;
	//if (_index == "0")
	//	int x = 1;

	childs.clear();
	//childs.shrink_to_fit();
	//for (int i = 0; i < FFTNODE_CHILDS_COUNT; ++i) {
	//	free(childs.at(i));
	//}
}

void FFTNode::InitChilds()
{
	int position = _begin;
	int nextPosition;
	int cnt = 0;
	for (int i = 0; i < FFTNODE_CHILDS_COUNT; ++i) 
	{
		nextPosition = position + windowSize;
		auto newNode = new FFTNode(controller, position, windowPow - 1, this, _index + String(cnt));
		childs.push_back(std::shared_ptr<FFTNode>(newNode));
		position = nextPosition;
		++cnt;
	}
}

void FFTNode::Recalc()
{
	auto wave = GetWave();
	auto fftComplex = PerformFFT(wave, windowSize);
	delete[] wave;
	RecalcFreqFunc(fftComplex);
}

void FFTNode::RecalcFreqFunc(juce::dsp::Complex<float>* data)
{
	auto frameSize = double(windowSize) / double(2);
	auto sampleRate = controller->GetSampleRate();
	auto binToFrequancy = double(sampleRate) / double(windowSize);
	for (auto bin = 0; bin < frameSize; bin++) 
	{
		auto real = data[bin].real();
		auto imag = data[bin].imag();
		auto phase = atan2(imag, real);
		auto magnitude = hypot(real, imag);
		auto frequancyActual = binToFrequancy * bin;
		func->AddPoint(frequancyActual, magnitude / frameSize);

		if (controller->GetTopPow() == windowPow)
			funcPhase->AddPoint(frequancyActual, phase);
	}
	func->Resort();
	//test
	auto max = func->GetMax();
}

float* FFTNode::GetWave()
{
	auto wave = new float[windowSize];
	auto sample = controller->GetSample();
	auto numSamples = sample.getNumSamples();
	if (numSamples == 0) return nullptr;
	auto buffer = sample.getReadPointer(0);

	int curPosition;
	for (int i = 0; i < windowSize; ++i) {
		curPosition = _begin + i;
		if (curPosition < numSamples)
			wave[i] = buffer[curPosition];
		else
			wave[i] = 0.0f;
	}
	return wave;
}


juce::dsp::Complex<float>* FFTNode::PerformFFT(float* wave, int size)
{
	juce::dsp::FFT forwardFFT(windowPow);
	juce::dsp::Complex<float>* waveComplex = new juce::dsp::Complex<float>[size];
	auto frameWave0 = PrepareDataComplex(wave, size, false);

	forwardFFT.perform(frameWave0, waveComplex, false);
	delete[] frameWave0;
	return waveComplex;
}

juce::dsp::Complex<float>* FFTNode::PrepareDataComplex(float* buffer, int windowsSize, bool useWindowFunction)
{
	juce::dsp::Complex<float>* compexData = new juce::dsp::Complex<float>[windowsSize];

	int curPosition;
	juce::dsp::Complex<float> temp;
	float value;
	for (auto i = 0; i < windowsSize; i++) {
		curPosition = i;
		value = buffer[curPosition];//
		//todo addwindow func

		compexData[i].real(value);
		compexData[i].imag(0);
	}
	return compexData;
}

ValuesFunc& FFTNode::GetSumFunc()
{
	if (parent) {
		if (!sumFunc) {
			auto parentFunc = parent->GetSumFunc();
			auto curFunc = *func;

			//auto stDev = curFunc.GetStandardDeviation();
			//curFunc.LimitingTheScope(stDev);
			auto treshold = curFunc.GetFuncTreshold();
			//curFunc.LimitingTheScope(treshold);

			sumFunc = std::shared_ptr<ValuesFunc>(curFunc + parentFunc);
		}
		return *sumFunc;
	}
	else {
		return GetFunc();
	}
}

ValuesFunc& FFTNode::GetFunc()
{
	return *func;
}


void FFTNode::AddLeaf(std::vector<std::shared_ptr<FFTNode>>* list)
{
	if (controller->GetBottomPow() == windowPow) 
	{
		if(!outOfRange)
			list->push_back(std::shared_ptr<FFTNode>(this));
	}
	else {
		for each (auto item in childs) {
			item->AddLeaf(list);
		}
	}
}


ValuesFunc& FFTNode::GetPhases() {
	return *funcPhase;
}


