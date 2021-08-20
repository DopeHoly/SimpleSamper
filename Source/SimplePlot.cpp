/*
  ==============================================================================

    SimplePlot.cpp
    Created: 24 Mar 2021 10:00:42pm
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include <algorithm>
#include "SimplePlot.h"

//==============================================================================
SimplePlot::SimplePlot()
{
}

SimplePlot::~SimplePlot()
{
}

void SimplePlot::paint (juce::Graphics& g)
{
    g.drawImage(mBufferImage, Rectangle<float>(0, 0, getWidth(), getHeight()));
}

void SimplePlot::resized()
{
    mBufferImage = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(mBufferImage);
    g.fillAll(backgroundColour);
    DrawOnImage(mBufferImage);
}

void SimplePlot::SetSample(std::vector<float>& points)
{
    mPoints.clear();
    mPoints = points;
    DrawOnImage(mBufferImage);
}


void SimplePlot::SetSample(std::vector<OSC_Setting>* points)
{
    mPoints2.clear();
    mPoints2 = *points;
    DrawOnImage(mBufferImage);
}

void SimplePlot::DrawOnImage(Image& image)
{
    Graphics g(image);
    g.fillAll(backgroundColour);
    Path curve;
    //const auto [min, max] = std::minmax_element(begin(mPoints), end(mPoints));
    auto max = 100;
    auto min = -200;
    double logMax = log10(22000);
    //const double freqCoef = (44100.0 / 2048.0);
    const double freqCoef = (44100.0 / double(2 << 12));
    curve.startNewSubPath(0, getHeight());
    for (int i = 0; i < mPoints.size() /2; ++i)
    {
        auto freq = freqCoef * i;
        auto x = i == 0 ? 0 : log10(freq);
        auto pointY = jmap<float>(Decibels::gainToDecibels(mPoints[i]), min, max, getHeight(), 0);
        auto pointX = jmap<float>(x, 0, logMax, 0, getWidth());

        curve.lineTo(pointX, pointY);
    }
    g.strokePath(curve, PathStrokeType(2));

    curve.startNewSubPath(0, getHeight());
    for (int i = 0; i < mPoints2.size(); ++i)
    {
        auto freq = mPoints2[i].freq;
        auto x = i == 0 ? 0 : log10(freq);
        auto pointY = jmap<float>(Decibels::gainToDecibels(mPoints2[i].vol), min, max, getHeight(), 0);
        auto pointX = jmap<float>(x, 0, logMax, 0, getWidth());

        if (pointX > 0 && pointY > 0)
        curve.lineTo(pointX, pointY);
    }

    g.strokePath(curve, PathStrokeType(2));
}
