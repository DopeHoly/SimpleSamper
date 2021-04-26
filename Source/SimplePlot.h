/*
  ==============================================================================

    SimplePlot.h
    Created: 24 Mar 2021 10:00:42pm
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"

//==============================================================================
/*
*/
class SimplePlot  : public juce::Component
{
public:
    SimplePlot();
    ~SimplePlot() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void SetSample(std::vector<float>& points);
    void SetSample(std::vector<OSC_Setting>* points);

private:

    std::vector<float> mPoints;
    std::vector<OSC_Setting> mPoints2;

    Image mBufferImage;

    Colour backgroundColor{ Colours::aliceblue.darker() };
    Colour lineColor{ Colours::white };

    void DrawOnImage(Image& image);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimplePlot)
};
