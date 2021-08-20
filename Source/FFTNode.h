#pragma once

#include <JuceHeader.h>
#include "ValuesFunc.h"

#define FFTNODE_CHILDS_COUNT 2

class Controller;


class FFTNode {
public:
	FFTNode(Controller* control, int begin, int pow, FFTNode* par, String index = "");
	~FFTNode();

	void InitChilds();
	void Recalc();
	void RecalcFreqFunc(juce::dsp::Complex<float>* data);
	float* GetWave();
	ValuesFunc& GetSumFunc();
	ValuesFunc& GetFunc();

	juce::dsp::Complex<float>* PerformFFT(float* wave, int size);

	juce::dsp::Complex<float>* PrepareDataComplex(float* buffer, int windowsSize, bool useWindowFunction);

	void AddLeaf(std::vector<std::shared_ptr<FFTNode>>* list);
	ValuesFunc& GetPhases();

private:
	Controller* controller;
	FFTNode* parent;
	std::vector<std::shared_ptr<FFTNode>> childs;
	std::shared_ptr<ValuesFunc> func;
	std::shared_ptr<ValuesFunc> funcPhase;
	std::shared_ptr<ValuesFunc> sumFunc;
	//std::shared_ptr <juce::dsp::Complex<double>> complexValue;

	int _begin{ 0 };
	int _end{ 0 };
	bool outOfRange{ false };
	int windowPow{0};
	int windowSize{0};
	String _index;
};