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
	fftw_complex* compexData = new fftw_complex[mWindowSize];

	int curPosition;
	fftw_complex temp;
	for (auto i = 0; i < mWindowSize; i++) {
		curPosition = position + i;
		temp[0] = 0;
		temp[0] = buffer[curPosition] * Gausse(i, mWindowSize);//windowHanning(i, N);
		temp[1] = 0;

		compexData[i][0] = temp[0];
		compexData[i][1] = temp[1];
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
	auto binToFrequancy = double(mSampleRate) / double(frameSize);
	auto winSum = windowsSum(mWindowSize);

 	for (auto bin = 0; bin < frameSize; bin++)
	{
		auto real0 = spec0[bin][0];
		auto imag0 = spec0[bin][1];
		auto real1 = spec1[bin][0];
		auto imag1 = spec1[bin][1];
		auto spectre0Magnitude = hypot(real0, imag0);
		auto spectre1Magnitude = hypot(real1, imag1);
		auto spectre0Phase = atan2(real0, imag0);
		auto spectre1Phase = atan2(real1, imag1);

		auto omegaExpected = DoublePi * (bin * binToFrequancy); // ω=2πf
		auto omegaActual = (spectre1Phase - spectre0Phase) / shiftTime; // ω=∂φ/∂t
		auto omegaDelta = Align(omegaActual - omegaExpected, DoublePi); // Δω=(∂ω + π)%2π - π
		auto binDelta = omegaDelta / (DoublePi * binToFrequancy);
		auto frequancyActual = (bin + binDelta) * binToFrequancy / 2.0;

		auto mag0 = 10 * log10(spectre0Magnitude);
		auto mag1 = 10 * log10(spectre1Magnitude);
		auto magnitude = spectre1Magnitude + spectre0Magnitude;
		magnitude *= 0.5 + abs(binDelta);
		magnitude *= 2.0 / winSum;

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
	myfile << "freq;vol;phase" << std::endl;
	auto cnt = dict.size();
	for (int i = 0; i < cnt; ++i) {
		auto item = dict[i];
		auto freq = std::to_string(item.freq);
		std::replace(freq.begin(), freq.end(), '.', ',');

		auto vol = std::to_string(item.vol);
		std::replace(vol.begin(), vol.end(), '.', ',');

		auto phase = std::to_string(item.phase);
		std::replace(phase.begin(), phase.end(), '.', ',');

		myfile << freq << ';' << vol << ';' << phase <<  std::endl;
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
		magnitude *= 2.0 / winSum;
		auto frequancyActual = binToFrequancy * bin;

		auto phase = spectre0Phase + ((3.0 * M_PI) / 4.0);

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

void SpectrumAnalyzer::CorrectPhase(float* sourceWave, int size, std::vector<OSC_Setting>& data)
{
	OSC_Setting* oscSetting = nullptr;
	float* wave;
	float* difwave;
	auto step = 0.01;

	double currentSum;
	double bestSum = INFINITY;
	double bestPhase;

	SimpleWaveGenerator waveGenerator (mSampleRate, 50000, 1);
	for (int j = 0; j < data.size(); ++j) 
	{
		oscSetting = &data[j];
		for (double i = -MathConstants<float>::pi; i < MathConstants<float>::pi; i += step) 
		{
			oscSetting->phase = i;

			waveGenerator.InitOscillators(data, j + 1);
			wave = waveGenerator.renderWave(size);
			difwave = DiffWave(sourceWave, wave, size);

			delete[] wave;

			currentSum = getSum(difwave, size);

			delete[] difwave;

			if (currentSum < bestSum) {
				bestSum = currentSum;
				bestPhase = i;
			}
		}

		oscSetting->phase = bestPhase;
	}

}

void SpectrumAnalyzer::CalcPhaseDif() 
{
	float* wave;
	fftw_plan p;
	auto step = 0.01;
	auto dictionary = std::make_unique<std::vector<OSC_Setting>>();
	SimpleWaveGenerator waveGenerator(mSampleRate, 50000, 1);
	waveGenerator.InitOscillators();
	waveGenerator.SetFrequency(440);

	for (double i = -MathConstants<float>::pi; i < MathConstants<float>::pi; i += step) {
		waveGenerator.SetPhase(i);

		wave = waveGenerator.renderWave(mWindowSize);

		fftw_complex* waveComplex = new fftw_complex[mWindowSize];
		auto frameWave0 = PrepareData(wave, mWindowSize, 0);
		delete[] wave;

		p = fftw_plan_dft_1d(mWindowSize, frameWave0, waveComplex, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
		auto resultWave = GetJoinedSpectrumSimple(waveComplex);

		fftw_free(waveComplex);
		fftw_free(frameWave0);

		auto maxFreq = findMaxFreq(*resultWave);
		maxFreq.freq = i;
		dictionary->push_back(maxFreq);
		resultWave->clear();
	}
	ExportFrequancyToCSV("D:\\YandexDisk\\phaseDif.csv", *dictionary);
}

std::unique_ptr<std::vector<OSC_Setting>> SpectrumAnalyzer::FFTSpectre()
{
	CalcPhaseDif();
	const bool oldFFT = true;

	auto startPos = mPosition;
	double ShiftsPerFrame = 32;//1.0 / 32.0;
	double step = double(mWindowSize) / ShiftsPerFrame;

	std::unique_ptr<std::vector<OSC_Setting>> result{ nullptr };


	if (oldFFT) {
		fftw_cleanup();
		fftw_plan p;
		fftw_complex* out0 = new fftw_complex[mWindowSize];
		fftw_complex* out1 = new fftw_complex[mWindowSize];

		auto sourceWave = GetSourceWave(startPos, mWindowSize + step);

		auto frame0 = PrepareData(startPos);
		auto frame1 = PrepareData(startPos + step);

		p = fftw_plan_dft_1d(mWindowSize, frame0, out0, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		p = fftw_plan_dft_1d(mWindowSize, frame1, out1, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		//result = GetJoinedSpectrum(out0, out1, ShiftsPerFrame);
		result = GetJoinedSpectrumSimple(out0);

		auto maxFreq = findMaxFreq(*result);

		auto sourcePeaks = FindPeaks(*result);
		//CorrectPhase(sourceWave, mWindowSize + step, *sourcePeaks);
		mWaveGenerator.InitOscillators(*sourcePeaks);

		//mWaveGenerator.SetGain(maxFreq.vol + 88.6);

		//mWaveGenerator.SetFrequency(maxFreq.freq);

		auto wave = mWaveGenerator.renderWave(mWindowSize + step);


		fftw_complex* wave0 = new fftw_complex[mWindowSize];
		fftw_complex* wave1 = new fftw_complex[mWindowSize];
		auto frameWave0 = PrepareData(wave, mWindowSize, 0);
		auto frameWave1 = PrepareData(wave, mWindowSize, 0 + step);

		p = fftw_plan_dft_1d(mWindowSize, frameWave0, wave0, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		p = fftw_plan_dft_1d(mWindowSize, frameWave1, wave1, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);

		auto resultWave = GetJoinedSpectrum(wave0, wave1, ShiftsPerFrame);
		auto maxFreqWave = findMaxFreq(*resultWave);
		auto gainMax = maxFreqWave.vol;

		//auto sourcePeaks = FindPeaks(*result);
		auto wavePeaks = FindPeaks(*resultWave);

		//ExportFrequancyToCSV("D:\\YandexDisk\\freq.csv", *result); 
		//ExportFrequancyToCSV("D:\\YandexDisk\\wave.csv", *resultWave);
		//ExportWaveToCSV("D:\\YandexDisk\\SourceWave.csv", sourceWave, mWindowSize + step);
		ExportWaveToCSV("D:\\YandexDisk\\ResultWave.csv", wave, mWindowSize + step);

		//result = Antialiasing(*result);
		delete[] out0;
		delete[] out1;
		delete[] frame0;
		delete[] frame1;
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
	mycomplex temp;
	for (auto i = 0; i < mWindowSize; i++) {
		curPosition = position + i;
		temp.r = 0;

		double window =
			0.5 - 0.5 * cos(2.0 * M_PI *
				i / mWindowSize);

		if (curPosition < numSamples)
			temp.r = buffer[curPosition] * Gausse(i, mWindowSize);//windowHanning(i, N);
		temp.i = 0;

		compexData[i].r = temp.r;
		compexData[i].i = temp.i;
	}
	return compexData;
}
