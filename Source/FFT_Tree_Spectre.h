#pragma once
#include <vector>

class FFT_Tree_Spectre {
public:
	FFT_Tree_Spectre();
	FFT_Tree_Spectre(int binNum, int binNumPhase, int freqNum, double timeToBin);
	~FFT_Tree_Spectre();
	void SetValue(int shot, int freqNum, double value);
	double GetValue(int shot, int freqNum);
	double GetPhase(int shot, int freqNum);
	void SetPhase(int bin, int freqNum, double value);

	double* spectre;
	double* frequancy;
	int size{ 0 };
	int numFrequeance{ 0 };
	int numBins{ 0 };
	int numBinsPhase{ 0 };
	double timeToBin{ 0.0 };
	double* phases;
	int difPow{ 0 };
};