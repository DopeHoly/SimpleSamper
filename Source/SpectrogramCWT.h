/*
  ==============================================================================

    SpectrogramCWT.h
    Created: 23 May 2021 3:20:19pm
    Author:  menin

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <wavelib.h>
#include <algorithm>
#include <tuple>
#include "FFT_Tree_Spectre.h"


struct ColourPos {
public:
    ColourPos();
    ColourPos(Colour _color, double pos);
    ~ColourPos();

    Colour color{ Colours::white };
    double position;
};

class Palete
{
public:
    Palete();
    ~Palete();

    void SetMax(double max) { _max = max; }
    void SetMin(double min) { _min = min; }

    double GetMax() { return _max; }
    double GetMin() { return _min; }

    void AddColour(ColourPos color);
    Colour GetColour(double value);

private:
    double _min;
    double _max;

    Colour blancColour{ Colours::white };
    std::vector< ColourPos> _colors;
};

class SpectrogramCWT : public juce::Component
{
public:
    SpectrogramCWT();
    ~SpectrogramCWT() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void SetSpectre(cwt_object wt);
    void SetSpectre(FFT_Tree_Spectre* data);
    void Clear();

private:

    //std::vector<float> mPoints;
    //std::vector<OSC_Setting> mPoints2;
    cwt_object _wt;
    FFT_Tree_Spectre* _spectre;
    bool dataLoaded{ false };

    Palete _palete;
    Image mBufferImage;
    Image mRealImage;

    Colour backgroundColour{ Colours::aliceblue.darker() };

    std::tuple<double, double>  GetMinMax(cplx_data* massive, int size);
    std::tuple<double, double>  GetMinMax(double* massive, int size);

    void DrawOnImageCWT(Image& image);
    void DrawOnImageFFT_TREE(Image& image);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrogramCWT)
};