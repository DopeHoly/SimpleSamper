#pragma once

#include "FFT_Tree_Spectre.h"

FFT_Tree_Spectre::FFT_Tree_Spectre():
	numFrequeance(0),
	numBins(0),
	size(0),
	spectre(nullptr),
	frequancy(nullptr)
{
}

FFT_Tree_Spectre::FFT_Tree_Spectre(int binNum, int binNumPhase, int freqNum, double timeToBin):
	numFrequeance(freqNum),
	numBins(binNum),
	size(binNum* freqNum),
	timeToBin(timeToBin),
	numBinsPhase(binNumPhase)
{
	spectre = new double[binNum * freqNum];
	phases = new double[binNumPhase * freqNum];
	frequancy = new double[freqNum];
}

FFT_Tree_Spectre::~FFT_Tree_Spectre()
{
	delete[] spectre;
	delete[] frequancy;
}


void FFT_Tree_Spectre::SetValue(int shot, int freqNum, double value)
{
	if (shot < 0 || shot > numBins)
		throw "Phase bin over range";

	if (freqNum < 0 || freqNum > numFrequeance)
		throw "Phase freq over range";

	spectre[shot * numFrequeance + freqNum] = value;
}

double FFT_Tree_Spectre::GetValue(int shot, int freqNum)
{
	if (shot < 0 || shot > numBins)
		throw "Phase bin over range";

	if (freqNum < 0 || freqNum > numFrequeance)
		throw "Phase freq over range";

	return spectre[shot * numFrequeance + freqNum];
}

double FFT_Tree_Spectre::GetPhase(int shot, int freqNum)
{
	auto lenght = 1 << difPow;
	auto bin = shot / lenght;
	if (bin < 0 || bin > numBinsPhase)
		throw "Phase bin over range";

	if (freqNum < 0 || freqNum > numFrequeance)
		throw "Phase freq over range";

	return phases[bin * numFrequeance + freqNum];
}

void FFT_Tree_Spectre::SetPhase(int bin, int freqNum, double value)
{
	if (bin < 0 || bin > numBinsPhase)
		throw "Phase bin over range";

	if (freqNum < 0 || freqNum > numFrequeance)
		throw "Phase freq over range";
	phases[bin * numFrequeance + freqNum] = value;
}
