#include <JuceHeader.h>
#include <fstream>
#include <string>
#include "SpectrumAnalyzer.h"
#include "PeakFinder.h"

SpectrumAnalyzer::SpectrumAnalyzer(double sampleRate, int samplesPerBlock):
	mSampleRate(sampleRate),
	mWaveGenerator(mSampleRate, samplesPerBlock, 1)
{
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
}

void SpectrumAnalyzer::SetSample(AudioSampleBuffer& sample)
{
	mSample = std::make_unique<AudioSampleBuffer>(sample);
	FFTSpectre();
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
	fftw_complex* out = new fftw_complex[mWindowSize];
	std::vector<float>  freqVal;
	//подготавливаем данные на вход
	auto in = PrepareData(mPosition);

	//формируем задание для fft
	p = fftw_plan_dft_1d(mWindowSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	fftw_execute(p);
	fftw_destroy_plan(p);
	double real, im;
	for (size_t i = 0; i < mWindowSize; i++)
	{
		real = out[i][0];
		im = out[i][1];
		freqVal.push_back(sqrt(real * real + im * im)/ mWindowSize);
	}
	delete[] out;
	delete[] in;
	return freqVal;
}

fftw_complex* SpectrumAnalyzer::PrepareData(int position)
{
	fftw_complex* compexData = new fftw_complex[mWindowSize];

	auto numSamples = mSample->getNumSamples();
	if (numSamples == 0) return nullptr;
	auto buffer = mSample->getReadPointer(0);

	int curPosition;
	fftw_complex temp;
	for (auto i = 0; i < mWindowSize; i++) {
		curPosition = position + i;
		temp[0] = 0;
		if (curPosition < numSamples)
			temp[0] = buffer[curPosition] * Gausse(i, mWindowSize);//windowHanning(i, N);
		temp[1] = 0;

		compexData[i][0] = temp[0];
		compexData[i][1] = temp[1];
	}
	return compexData;
}

fftw_complex* SpectrumAnalyzer::PrepareData(float* buffer, int windowsSize, int position)
{
	fftw_complex* compexData = new fftw_complex[windowsSize];

	int curPosition;
	fftw_complex temp;
	for (auto i = 0; i < windowsSize; i++) {
		curPosition = position + i;
		auto value = buffer[curPosition];// *Gausse(i, windowsSize);

		if (UseWindow)
			value *= Gausse(i, mWindowSize);

		compexData[i][0] = value;
		compexData[i][1] = 0;
	}
	return compexData;
}

juce::dsp::Complex<float>* SpectrumAnalyzer::PrepareDataJuceComplex(float* buffer, int windowsSize, int position)
{
	juce::dsp::Complex<float>* compexData = new juce::dsp::Complex<float>[mWindowSize];

	int curPosition;
	juce::dsp::Complex<float> temp;
	for (auto i = 0; i < mWindowSize; i++) {
		curPosition = position + i;
		temp.real(0);
		temp.real(buffer[curPosition] * Gausse(i, mWindowSize));//windowHanning(i, N);
		temp.imag(0);

		compexData[i].real(temp.real());
		compexData[i].imag(temp.imag());
	}
	return compexData;
}

double SpectrumAnalyzer::windowHanning(double n, double frameSize) {
	return 0.5 * (1 - cos((2 * M_PI * n) / (frameSize - 1)));
}

double SpectrumAnalyzer::windowsSum(double frameSize) {
	auto sum = 0.0f;
	for (int i = 0; i < frameSize; ++i) {
		sum += Gausse(i, frameSize);
	}
	return sum;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrum(fftw_complex* spec0, fftw_complex* spec1, double shiftsPerFrame)
{
	auto DoublePi = M_PI * 2;
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	auto frameSize = mWindowSize / 2;
	auto frameTime = double(frameSize) / double(mSampleRate);
	auto shiftTime = frameTime / shiftsPerFrame; 
	auto binToFrequancy = double(mSampleRate) / double(mWindowSize);
	auto winSum = windowsSum(mWindowSize);

 	for (auto bin = 0; bin < frameSize; bin++)
	{
		auto real0 = spec0[bin][0];
		auto imag0 = spec0[bin][1];
		auto real1 = spec1[bin][0];
		auto imag1 = spec1[bin][1];
		auto spectre0Magnitude = hypot(real0, imag0);
		auto spectre1Magnitude = hypot(real1, imag1);
		auto spectre0Phase = atan2(imag0,real0);
		auto spectre1Phase = atan2(imag1, real1);

		auto omegaExpected = DoublePi * (bin * binToFrequancy); // ω=2πf
		auto omegaActual = (spectre1Phase - spectre0Phase) / shiftTime; // ω=∂φ/∂t
		auto omegaDelta = Align(omegaActual - omegaExpected, DoublePi); // Δω=(∂ω + π)%2π - π
		auto binDelta = omegaDelta / (DoublePi * binToFrequancy);
		auto frequancyActual = (bin + binDelta) * binToFrequancy;

		auto mag0 = 10 * log10(spectre0Magnitude);
		auto mag1 = 10 * log10(spectre1Magnitude);
		auto magnitude = spectre1Magnitude + spectre0Magnitude;
		magnitude *= 0.5 + abs(binDelta);
		//magnitude *= 2.0 / winSum;

		if (trunc(frequancyActual) == 440) {
			auto stop = 1;
		}

		dictionary->push_back(OSC_Setting((bin == 0) ? 1 : frequancyActual, 
			//magnitude /( 10 * log10(frameSize / 2.0f))/*magnitude*/
			20.0 * log10(magnitude / frameSize),
			fmod(-binDelta, M_PI)
		));
	}

	//double max = -INFINITY;
	//for each (auto item in *dictionary)
	//{
	//	if (max < item.vol)
	//		max = item.vol;
	//}

	//for(int i = 0; i < (*dictionary).size(); ++i)
	//{
	//	(*dictionary)[i].vol = 20 * log10((*dictionary)[i].vol / max);
	//}


	return dictionary;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrum(mycomplex* spec0, mycomplex* spec1, double shiftsPerFrame)
{
	auto DoublePi = M_PI * 2;
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	auto frameSize = mWindowSize / 2;
	double step = (double)frameSize / shiftsPerFrame;
	double fps = (double)mSampleRate / (double)frameSize;
	double expect = 2.0 * M_PI * step / (double)frameSize;

	for (auto bin = 0; bin < frameSize; bin++)
	{
		auto real0 = spec0[bin].r;
		auto imag0 = spec0[bin].i;
		auto real1 = spec1[bin].r;
		auto imag1 = spec1[bin].i;
		auto s0Magnitude = hypot(real0, imag0);
		auto s1Magnitude = hypot(real1, imag1);
		auto s0Phase = atan2(real0, imag0);
		auto s1Phase = atan2(real1, imag1);

		auto deltaPhase = s0Phase - s1Phase;
		deltaPhase -= (double)bin * expect;
		int qpd = deltaPhase / M_PI;

		if (qpd >= 0)
			qpd += qpd & 1;

		else
			qpd -= qpd & 1;

		deltaPhase -= M_PI * (double)qpd;

		auto omegaDelta = Align(deltaPhase, DoublePi);

		double df = deltaPhase * shiftsPerFrame / (DoublePi);
		double frequancyActual = (bin + df) * fps;
		double SumMagnitude = s1Magnitude + s0Magnitude;
		double magnitude = SumMagnitude * (0.5 + abs(df));

		dictionary->push_back(OSC_Setting((bin == 0)? 1 : frequancyActual , magnitude, df));
	}

	return dictionary;
}

bool SpectrumAnalyzer::ExportFrequancyToCSV(String path, std::vector<OSC_Setting>& dict)
{
	auto file = File(path);
	std::ofstream myfile;
	myfile.open(path.toStdString());
	myfile << "freq;vol;phase;re;im" << std::endl;
	auto cnt = dict.size();
	for (int i = 0; i < cnt; ++i) {
		auto item = dict[i];
		auto freq = std::to_string(item.freq);
		std::replace(freq.begin(), freq.end(), '.', ',');

		auto vol = std::to_string(item.vol);
		std::replace(vol.begin(), vol.end(), '.', ',');

		auto phase = std::to_string(item.phase);
		std::replace(phase.begin(), phase.end(), '.', ',');

		auto re = std::to_string(item.re);
		std::replace(re.begin(), re.end(), '.', ',');

		auto im = std::to_string(item.im);
		std::replace(im.begin(), im.end(), '.', ',');

		myfile << freq << ';' << vol << ';' << phase << ';' << re << ';' << im << std::endl;
	}
	myfile.close();
	return true;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrumSimple(fftw_complex* spec0)
{
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	auto frameSize = mWindowSize / 2;
	auto frameTime = double(frameSize) / double(mSampleRate);
	auto binToFrequancy = double(mSampleRate) / double(mWindowSize);
	auto winSum = windowsSum(mWindowSize);
	for (auto bin = 0; bin < frameSize; bin++) {
		auto real0 = spec0[bin][0];
		auto imag0 = spec0[bin][1];
		auto spectre0Magnitude = hypot(real0, imag0);
		auto spectre0Phase = atan2(imag0, real0);
		auto magnitude = spectre0Magnitude;

		if (UseWindow)
			magnitude *= 2.0 / winSum;
		auto frequancyActual = binToFrequancy * bin;

		auto phase = spectre0Phase;

		//if (real0 < 0) {
		//	if (imag0 > 0)
		//		phase += -MathConstants<double>::pi;
		//	else
		//	if (imag0 < 0)
		//		phase += MathConstants<double>::pi;
		//}

		dictionary->push_back(OSC_Setting((bin == 0) ? 0.01 : frequancyActual,
			20.0 * log10(magnitude / frameSize),
			phase,
			real0,
			imag0
		));
	}
	return dictionary;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrumSimple(mycomplex* spec0)
{
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	auto frameSize = mWindowSize / 2;
	auto frameTime = double(frameSize) / double(mSampleRate);
	auto binToFrequancy = double(mSampleRate) / double(mWindowSize);
	auto winSum = windowsSum(mWindowSize);
	for (auto bin = 0; bin < frameSize; bin++) {
		auto real0 = spec0[bin].i;
		auto imag0 = spec0[bin].r;
		auto spectre0Magnitude = hypot(real0, imag0);
		auto spectre0Phase = atan2(imag0, real0);
		auto magnitude = spectre0Magnitude;
		if (UseWindow)
			magnitude *= 2.0 / winSum;
		auto frequancyActual = binToFrequancy * bin;

		auto phase = spectre0Phase;

		//if (real0 < 0) {
		//	if (imag0 > 0)
		//		phase += -MathConstants<double>::pi;
		//	else
		//	if (imag0 < 0)
		//		phase += MathConstants<double>::pi;
		//}

		dictionary->push_back(OSC_Setting(/*(bin == 0) ? 0.01 : */frequancyActual,
			//20.0 * log10(magnitude / frameSize),
			magnitude,
			phase,
			real0,
			imag0
		));
	}
	return dictionary;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrumSimple(juce::dsp::Complex<float>* spec0)
{
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	auto frameSize = mWindowSize / 2;
	auto frameTime = double(frameSize) / double(mSampleRate);
	auto binToFrequancy = double(mSampleRate) / double(mWindowSize);
	auto winSum = windowsSum(mWindowSize);
	for (auto bin = 0; bin < frameSize; bin++) {
		auto real0 = spec0[bin].real();
		auto imag0 = spec0[bin].imag();
		auto spectre0Magnitude = hypot(real0, imag0);
		auto spectre0Phase = atan2(imag0, real0);
		auto magnitude = spectre0Magnitude;

		if (UseWindow)
			magnitude *= 2.0 / winSum;
		auto frequancyActual = binToFrequancy * bin;

		auto phase = spectre0Phase;

		if (trunc(frequancyActual) == 440) {
			auto stop = 1;
		}

		dictionary->push_back(OSC_Setting((bin == 0) ? 0.01 : frequancyActual,
			20.0 * log10(magnitude / frameSize),
			phase
		));
	}
	return dictionary;
}

bool SpectrumAnalyzer::ExportWaveToCSV(String path, float* data, int size, int startPos)
{
	auto file = File(path);
	std::ofstream myfile;
	myfile.open(path.toStdString());
	myfile << "vol" << std::endl;
	auto cnt = startPos + size;
	for (int i = startPos; i < cnt; ++i) {
		auto value = std::to_string(data[i]);
		std::replace(value.begin(), value.end(), '.', ',');

		myfile << value << std::endl;
	}
	myfile.close();
	return true;
}

bool SpectrumAnalyzer::ExportComplexToCSV(String path, fftw_complex* data, int size)
{
	auto file = File(path);
	std::ofstream myfile;
	myfile.open(path.toStdString());
	myfile << "real;imag" << std::endl;
	for (int i = 0; i < size; ++i) {

		auto re = std::to_string(data[i][0]);
		auto im = std::to_string(data[i][1]);
		std::replace(re.begin(), re.end(), '.', ',');
		std::replace(im.begin(), im.end(), '.', ',');

		myfile << re << ";" << im << std::endl;
	}
	myfile.close();
	return true;
}

bool SpectrumAnalyzer::ExportComplexToCSV(String path, mycomplex* data, int size)
{
	auto file = File(path);
	std::ofstream myfile;
	myfile.open(path.toStdString());
	myfile << "real;imag" << std::endl;
	for (int i = 0; i < size; ++i) {

		auto re = std::to_string(data[i].r);
		auto im = std::to_string(data[i].i);
		std::replace(re.begin(), re.end(), '.', ',');
		std::replace(im.begin(), im.end(), '.', ',');

		myfile << re << ";" << im << std::endl;
	}
	myfile.close();
	return true;
}

bool SpectrumAnalyzer::ExportComplexToCSV(String path, juce::dsp::Complex<float>* data, int size)
{
	auto file = File(path);
	std::ofstream myfile;
	myfile.open(path.toStdString());
	myfile << "real;imag" << std::endl;
	for (int i = 0; i < size; ++i) {

		auto re = std::to_string(data[i].real());
		auto im = std::to_string(data[i].imag());
		std::replace(re.begin(), re.end(), '.', ',');
		std::replace(im.begin(), im.end(), '.', ',');

		myfile << re << ";" << im << std::endl;
	}
	myfile.close();
	return true;
}

float* SpectrumAnalyzer::GetSourceWave(int position, int windowsSize) {
	auto wave = new float[windowsSize];

	auto numSamples = mSample->getNumSamples();
	if (numSamples == 0) return nullptr;
	auto buffer = mSample->getReadPointer(0);

	int curPosition;
	for (int i = 0; i < windowsSize; ++i) {
		curPosition = position + i;
		if (curPosition < numSamples)
			wave[i] = buffer[curPosition];
		else
			wave[i] = 0.0f;
	}
	return wave;
}

OSC_Setting& SpectrumAnalyzer::findMaxFreq(std::vector<OSC_Setting>& dict)
{
	auto maxDB = -DBL_MAX;
	OSC_Setting* max{ nullptr };
	auto cnt = dict.size();
	for (int i = 0; i < cnt; ++i) {
		auto item = &dict[i];
		if (maxDB < item->vol) {
			maxDB = item->vol;
			max = item;
		}
	}
	return *max;
}

int SpectrumAnalyzer::findMaxFreqId(std::vector<OSC_Setting>& dict)
{
	auto maxDB = -DBL_MAX;
	//OSC_Setting* max{ nullptr };
	int maxId = -1;
	auto cnt = dict.size();
	for (int i = 0; i < cnt; ++i) {
		auto item = &dict[i];
		if (maxDB < item->vol) {
			maxDB = item->vol;
			maxId = i;
		}
	}
	return maxId;
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::Antialiasing(std::vector<OSC_Setting>& data)
{
	auto result = std::make_unique<std::vector<OSC_Setting>>();
	auto cnt = data.size();
	for (int j = 0; j < cnt - 4; ++j) {
		auto i = j;
		auto x0 = data[i].freq;
		auto x1 = data[i + 1].freq;
		auto y0 = data[i].vol;
		auto y1 = data[i + 1].vol;

		auto a = (y1 - y0) / (x1 - x0);
		auto b = y0 - a * x0;

		i += 2;
		auto u0 = data[i].freq;
		auto u1 = data[i + 1].freq;
		auto v0 = data[i].vol;
		auto v1 = data[i + 1].vol;

		auto c = (v1 - v0) / (u1 - u0);
		auto d = v0 - c * u0;

		auto x = (d - b) / (a - c);
		auto y = (a * d - b * c) / (a - c);

		if (y > y0 && y > y1 && y > v0 && y > v1 &&
			x > x0 && x > x1 && x < u0 && x < u1)
		{
			result->push_back(OSC_Setting(x1, y1));
			result->push_back(OSC_Setting(x, y));
			//result.Add(x1, y1);
			//result.Add(x, y);
		}
		else
		{
			result->push_back(OSC_Setting(x1, y1));
			//result.Add(x1, y1);
		}
	}
	return result;
}

double SpectrumAnalyzer::Gausse(double n, double frameSize)
{
	const double Q = 0.5;
	auto a = (frameSize - 1) / 2;
	auto t = (n - a) / (Q * a);
	t = t * t;
	return exp(-t / 2);
}


std::unique_ptr<std::vector<OSC_Setting>> FindPeaks(std::vector<OSC_Setting>& data) {

	std::vector<float> in;
	std::vector<int> out;

	auto result = std::make_unique<std::vector<OSC_Setting>>();

	for each (auto item in data)
	{
		in.push_back(item.vol);
	}
	PeakFinder::findPeaks(in, out);

	for each (auto id in out)
	{
		result->push_back(data[id]);
	}
	return result;
}

double getSum(float* data, int size)
{
	double sum = 0.0;
	for (int i = 0; i < size; ++i) {
		sum += data[i];
	}
	return sum;
}

float* DiffWave(float* a, float* b, int size) {
	auto difWave = new float[size];
	for (int i = 0; i < size; ++i) {
		difWave[i] = ( abs(a[i] - b[i]) );
	}
	return difWave;
}

void SpectrumAnalyzer::CorrectPhase(std::vector<OSC_Setting>& data)
{
	float* wave;
	fftw_plan p;
	OSC_Setting* oscSetting = nullptr;
	double bestPhase;
	std::unique_ptr<std::vector<OSC_Setting>> resultWave;
	auto defaultPhase = -MathConstants<float>::pi;
	SimpleWaveGenerator waveGenerator(mSampleRate, 50000, 1);
	waveGenerator.InitOscillators();
	waveGenerator.SetPhase(defaultPhase);

	for (int j = 0; j < data.size(); ++j) 
	{
		oscSetting = &data[j];
		waveGenerator.SetFrequency(trunc(oscSetting->freq));
		wave = waveGenerator.renderWave(mWindowSize);

		//oscSetting->phase = bestPhase; 
		fftw_complex* waveComplex = new fftw_complex[mWindowSize];
		auto frameWave0 = PrepareData(wave, mWindowSize, 0);
		p = fftw_plan_dft_1d(mWindowSize, frameWave0, waveComplex, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
		resultWave = GetJoinedSpectrumSimple(waveComplex);
		auto maxFreq = findMaxFreq(*resultWave);
		auto phaseDif = defaultPhase - maxFreq.phase;
		auto curentPhase = oscSetting->phase + phaseDif;
		auto minusnormalizePhase = curentPhase < -MathConstants<float>::pi ? curentPhase + MathConstants<float>::twoPi : curentPhase;
		auto normalizePhase = minusnormalizePhase > MathConstants<float>::pi ? minusnormalizePhase - MathConstants<float>::twoPi : minusnormalizePhase;
		oscSetting->phase = normalizePhase;

		delete[] wave;
		fftw_free(waveComplex);
		fftw_free(frameWave0);
		resultWave->clear();
	}
}

void SpectrumAnalyzer::CalcPhaseDif() 
{
	bool useJuceFFT = true;

	FFTMethod curentMethod = Mine;

	float* wave;
	fftw_plan p;
	//auto step = 0.01;

	double ShiftsPerFrame = 32;//1.0 / 32.0;
	double step = double(mWindowSize) / ShiftsPerFrame;

	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	SimpleWaveGenerator waveGenerator(mSampleRate, 50000, 1);
	waveGenerator.InitOscillators();
	//waveGenerator.SetFrequency(1000);
	waveGenerator.SetGain(0.25);

	waveGenerator.SetPhase(0);

	juce::dsp::FFT forwardFFT(mWindowSize);
	std::unique_ptr<std::vector<OSC_Setting>> resultWave;

	//for (double i = -MathConstants<float>::pi; i <= MathConstants<float>::pi; i += step) {
		//waveGenerator.SetPhase(i);
	for (double i = 20; i <= 2000; ++i ) {
		//waveGenerator.SetPhase(i);
		waveGenerator.SetFrequency(i);
		wave = waveGenerator.renderWave(mWindowSize);
		//wave = waveGenerator.renderWave(mWindowSize + step);

		switch (curentMethod)
		{
		case Juce:
		{
			juce::dsp::Complex<float>* waveComplex = PerformFFTJuce(wave, mWindowSize);
			resultWave = GetJoinedSpectrumSimple(waveComplex);
			delete[] waveComplex;
		}
			break;
		case FFTW: 
		{
			fftw_complex* waveComplex = PerformFFT(wave, mWindowSize);
			resultWave = GetJoinedSpectrumSimple(waveComplex);
			fftw_free(waveComplex);
		}
			break;
		case Mine:
		{
			auto waveComplex = PerformFFT2(wave, mWindowSize);
			resultWave = GetJoinedSpectrumSimple(waveComplex);
			delete[] waveComplex;
		}
			break;
		case Overlap:
		{
			delete[] wave;
			wave = waveGenerator.renderWave(mWindowSize + step);
			//auto waveComplex = GetJoinedSpectrumWave(wave, mWindowSize, ShiftsPerFrame);
			resultWave = GetJoinedSpectrumWave(wave, mWindowSize, ShiftsPerFrame);
			//delete[] waveComplex;
		}
			break;
		default:
			break;
		}

		delete[] wave;

		auto maxFreq = findMaxFreq(*resultWave);
		//maxFreq.freq = i;
		//maxFreq.re = GetCenterMassXMaxFreq(*resultWave);
		dictionary->push_back(maxFreq);
		resultWave->clear();
	}
	//CorrectPhase(*dictionary);
	ExportFrequancyToCSV("D:\\YandexDisk\\phaseDif.csv", *dictionary);
	dictionary->clear();
}


fftw_complex* SpectrumAnalyzer::PerformFFT(float* wave, int size, int position) {
	fftw_plan p;
	fftw_complex* waveComplex = new fftw_complex[size];
	auto frameWave4 = PrepareData(wave, size, position);
	p = fftw_plan_dft_1d(size, frameWave4, waveComplex, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_execute(p);

	fftw_destroy_plan(p);
	fftw_free(frameWave4);

	return waveComplex;
}

mycomplex* SpectrumAnalyzer::PerformFFT2(float* wave, int size, int position)
{
	auto complexData = PreapareComplex(wave, size, position);
	fftr(complexData, size);
	return complexData;
}

juce::dsp::Complex<float>* SpectrumAnalyzer::PerformFFTJuce(float* wave, int size, int position)
{
	juce::dsp::FFT forwardFFT(winPow + 1);
	juce::dsp::Complex<float>* waveComplex = new juce::dsp::Complex<float>[size];
	auto frameWave0 = PrepareDataJuceComplex(wave, size, position);

	forwardFFT.perform(frameWave0, waveComplex, false);
	delete[] frameWave0;
	return waveComplex;
}


std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::GetJoinedSpectrumWave(float* wave, int size, int ShiftsPerFrame)
{
	double step = double(size) / double(ShiftsPerFrame);

	auto waveComplex1 = PerformFFT(wave, size, 0);
	auto waveComplex2 = PerformFFT(wave, size, step);

	auto result = GetJoinedSpectrum(waveComplex1, waveComplex2, step);
	fftw_free(waveComplex1);
	fftw_free(waveComplex2);
	return result;
}


std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::FFTSpectre()
{
	CalcPhaseDif();
	const bool oldFFT = true;

	auto startPos = mPosition;
	double ShiftsPerFrame = 32;//1.0 / 32.0;
	double step = double(mWindowSize) / ShiftsPerFrame;

	std::unique_ptr<std::vector<OSC_Setting>> result{ nullptr };
	SimpleWaveGenerator waveGenerator(mSampleRate, 50000, 1);
	waveGenerator.InitOscillators();
	waveGenerator.SetFrequency(5000);
	waveGenerator.SetGain(0.25);
	waveGenerator.SetPhase(0);

	if (oldFFT) {
		fftw_cleanup();
		fftw_plan p;
		fftw_complex* out0 = new fftw_complex[mWindowSize];
		fftw_complex* out1 = new fftw_complex[mWindowSize];

		auto renderWave = waveGenerator.renderWave(mWindowSize);

		auto sourceWave = GetSourceWave(startPos, mWindowSize/* + step*/);


		ExportWaveToCSV("D:\\YandexDisk\\SourceWave.csv", sourceWave, mWindowSize);
		ExportWaveToCSV("D:\\YandexDisk\\RenderWave.csv", renderWave, mWindowSize);

		auto complexRender = PerformFFT(renderWave, mWindowSize);
		//CorrectPhaseActive = true;
		auto complexSource = PerformFFT(sourceWave, mWindowSize);

		//ExportComplexToCSV("D:\\YandexDisk\\complexRender.csv", complexRender, mWindowSize);
		//ExportComplexToCSV("D:\\YandexDisk\\complexSource.csv", complexSource, mWindowSize);

		auto resultRender = GetJoinedSpectrumSimple(complexRender);
		auto resultSource = GetJoinedSpectrumSimple(complexSource);

		//result = GetJoinedSpectrum(out0, out1, ShiftsPerFrame);
		//result = GetJoinedSpectrumSimple(out0);

		auto maxFreqRender = FindPeaks(*resultRender);
		auto maxFreqSource = FindPeaks(*resultSource);
		//ExportFrequancyToCSV("D:\\YandexDisk\\Renderpectre.csv", *result);

		//CorrectPhase(*maxFreqRender);
		//CorrectPhase(*maxFreqSource);

		auto sourcePeaks = FindPeaks(*result);

		
	}
	else {
		auto frame0 = PreapareComplex(startPos);
		auto frame1 = PreapareComplex(startPos + step);

		fftr(frame0, mWindowSize);
		fftr(frame1, mWindowSize);
		result = GetJoinedSpectrum(frame0, frame1, ShiftsPerFrame);
		delete[] frame0;
		delete[] frame1;
	}

	//ExportToCSV("D:\\YandexDisk\\freq.csv", *result);
	auto maxFreq = findMaxFreq(*result);

	return result; 
}


double SpectrumAnalyzer::Align(double angle, double period)
{
	auto qpd = (int)(angle / period);
	if (qpd >= 0) qpd += qpd & 1;
	else qpd -= qpd & 1;
	angle -= period * qpd;
	return angle;
}


void SpectrumAnalyzer::fftr(mycomplex a[], int n)
{
	double norm = sqrt(1.0 / n);

	for (int i = 0, j = 0; i < n; i++)
	{
		if (j >= i)
		{
			double tr = a[j].r * norm;

			a[j].r = a[i].r * norm;
			a[j].i = 0.0;

			a[i].r = tr;
			a[i].i = 0.0;
		}

		int m = n / 2;
		while (m >= 1 && j >= m)
		{
			j -= m;
			m /= 2;
		}
		j += m;
	}

	for (int mmax = 1, istep = 2 * mmax; mmax < n;
		mmax = istep, istep = 2 * mmax)
	{
		double delta = (M_PI / mmax);
		for (int m = 0; m < mmax; m++)
		{
			double w = m * delta;
			double wr = cos(w);
			double wi = sin(w);

			for (int i = m; i < n; i += istep)
			{
				int j = i + mmax;
				double tr = wr * a[j].r - wi * a[j].i;
				double ti = wr * a[j].i + wi * a[j].r;
				a[j].r = a[i].r - tr;
				a[j].i = a[i].i - ti;
				a[i].r += tr;
				a[i].i += ti;
			}
		}
	}
}

mycomplex* SpectrumAnalyzer::PreapareComplex(int position) {

	mycomplex* compexData = new mycomplex[mWindowSize];

	auto numSamples = mSample->getNumSamples();
	if (numSamples == 0) return nullptr;
	auto buffer = mSample->getReadPointer(0);

	int curPosition;
	double real;
	for (auto i = 0; i < mWindowSize; i++) {
		curPosition = position + i;
		real = 0;

		if (curPosition < numSamples)
			real = buffer[curPosition];

		if (UseWindow)
			real *= Gausse(i, mWindowSize);

		compexData[i].r = real;
		compexData[i].i = 0;
	}
	return compexData;
}

mycomplex* SpectrumAnalyzer::PreapareComplex(float* buffer, int windowsSize, int position)
{
	mycomplex* compexData = new mycomplex[windowsSize];

	int curPosition;
	double real;
	for (auto i = 0; i < windowsSize; i++) {
		curPosition = position + i;

		real = buffer[curPosition];
		if(UseWindow)
			real *= Gausse(i, mWindowSize);

		compexData[i].r = real;
		compexData[i].i = 0;
	}
	return compexData;
}


//хуйня
#pragma region CenterMass

void intersect(double a1, double a2, double b1, double b2, double c1, double c2, double& x, double& y)
{
	double det = a1 * b2 - a2 * b1;
	x = (b1 * c2 - b2 * c1) / det;
	y = (a2 * c1 - a1 * c2) / det;
}

void LinePosition(double& x, double& y, double x1, double y1, double x2, double y2, double  x3, double y3, double x4, double y4)
{
	double a1, a2, b1, b2, c1, c2;
	// line equation ax + by + c = 0
	a1 = y1 - y2;
	b1 = x2 - x1;
	c1 = x1 * y2 - x2 * y1;
	a2 = y3 - y4;
	b2 = x4 - x3;
	c2 = x3 * y4 - x4 * y3;

	//parall(a1, a2, b1, b2);(a1 / a2) == (b1 / b2)

	intersect(a1, a2, b1, b2, c1, c2, x, y);
}

//Center mass of sqrTriangle by 0X
void CenterMass(double& x, double& y, double x1, double y1, double x2, double y2)
{
	auto dy = y2 - y1;
	auto dx = x2 - x1;
	if (dy == 0) {
		x = x1 + dx / 2.0;
		y = y1;
		return;
	}

	if (dy > 0) //leftside
	{
		LinePosition(x, y, x1, y1, x2, y1 + dy / 2.0, x2, y2, x1 + dx / 2.0, y1);
	}
	else //rightside
	{
		LinePosition(x, y, x1, y1, x1 + dx / 2.0, y2, x2, y2, x1, y1 + dy / 2.0);
	}
}

double CenterMassX(std::vector<OSC_Setting>& data, int idBegin, int idEnd)
{
	jassert(data.size() > idEnd);

	double tmpX = 0;
	double tmpY = 0;
	double sumX = 0;
	for (auto i = idBegin; i < idEnd; ++i)
	{
		auto curent = data.at(i);
		auto next = data.at(i + 1);
		CenterMass(tmpX, tmpY, curent.freq, curent.vol, next.freq, next.vol);
		sumX += tmpX;
	}
	return sumX / double(idEnd - idBegin);
}

double SpectrumAnalyzer::GetCenterMassXMaxFreq(std::vector<OSC_Setting>& data)
{
	//ExportFrequancyToCSV("D:\\YandexDisk\\TestData.csv", data);
	int maxId = findMaxFreqId(data);
	int beginId = -1;
	int endId = -1;

	int curId = maxId;
	auto curData = &data.at(curId);
	OSC_Setting* nextData;
	double dif;
	//left
	while (curId - 1 >= 0) {
		nextData = &data.at(curId - 1);
		dif = curData->vol - nextData->vol;
		if (dif <= 0) {
			beginId = curId;
			break;
		}
		--curId;
		curData = nextData;
	}

	curId = maxId;
	curData = &data.at(curId);
	//right
	while (curId + 1 < data.size()) {
		nextData = &data.at(curId + 1);
		dif = curData->vol - nextData->vol;
		if (dif <= 0) {
			endId = curId;
			break;
		}
		++curId;
		curData = nextData;
	}

	beginId = (beginId == -1) ? 0 : beginId;
	endId = (endId == -1) ? data.size() - 1 : endId;

	return CenterMassX(data, beginId, endId);
}

#pragma endregion