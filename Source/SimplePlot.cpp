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
    g.fillAll(backgroundColor);
    DrawOnImage(mBufferImage);
}

void SimplePlot::SetSample(std::vector<float>& points)
{
    mPoints.clear();
    mPoints = points;
    DrawOnImage(mBufferImage);
}

void SimplePlot::DrawOnImage(Image& image)
{
    Graphics g(image);
    g.fillAll(backgroundColor);
    Path curve;
    const auto [min, max] = std::minmax_element(begin(mPoints), end(mPoints));

    curve.startNewSubPath(0, getHeight());
    for (int i = 0; i < mPoints.size(); ++i)
    {
        auto point = jmap<float>(mPoints[i], *min, *max, getHeight(), 0);
        curve.lineTo(i, point);
    }
    g.strokePath(curve, PathStrokeType(2));
}
