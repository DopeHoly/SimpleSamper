#include "MyPeakFinder.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

void mydiff(std::vector<float> in, std::vector<float>& out)
{
	out = std::vector<float>(in.size() - 1);

	for (int i = 1; i < in.size(); ++i)
		out[i - 1] = in[i] - in[i - 1];
}

double MaxElement(std::vector<float> x0) 
{
	double max = -INFINITY;
	for each (auto item in x0) {
		if (max < item) {
			max = item;
		}
	}
	return max;
}

double MinElement(std::vector<float> x0)
{
	double min = INFINITY;
	for each (auto item in x0) {
		if (min > item) {
			min = item;
		}
	}
	return min;
}

template<typename T>
T GetTreshold(std::vector<T> x0) {
	auto min = MinElement(x0);
	auto max = MaxElement(x0);
	auto dif = max - min;

	int N = 50;

	double step = dif / double(N);
	double begin = min;
	double end = begin + step;

	int maxIndex = -1;
	int maxValue = -INFINITY;
	for (int i = 0; i < N; ++i)
	{
		auto cnt = 0;
		for each (auto item in x0)
		{
			if (begin <= item && item < end) {
				++cnt;
			}
		}
		if (cnt > maxValue) {
			maxValue = cnt;
			maxIndex = i;
		}
		begin = end;
		end += step;
	}

	return min + step * (maxIndex + 1);
}

void myfindPeaks(std::vector<float> x0, std::vector<int>& peakInds)
{
	std::vector<float> dx;
	mydiff(x0, dx);
	auto size = x0.size() - 1;
	double treshold = GetTreshold<float>(x0);
	for (int i = 1; i < size; ++i) {
		auto xPrev = x0.at(i - 1);
		auto x = x0.at(i);
		auto dxPrev = dx.at(i - 1);
		auto dxCur = dx.at(i);
		if (dxPrev > 0 && dxCur <= 0) {
			if (dxCur == 0) {
				if (x > treshold) {
					peakInds.push_back(i);
				}
			}
			else {
				if (xPrev > treshold) {
					peakInds.push_back(i);
				}
			}
		}
	}
}
