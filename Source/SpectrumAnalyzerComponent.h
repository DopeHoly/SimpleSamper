/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 22 Mar 2021 6:59:06am
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SimplePlot.h"
#include "SpectrumAnalyzer.h"


enum
{
    fftOrder = 11,
    fftSize = 1 << fftOrder,
    scopeSize = 512
};

//==============================================================================
/*
*/
class SpectrumAnalyzerComponent  : public juce::Component,
                                   public Slider::Listener
{
public:
    SpectrumAnalyzerComponent();
    ~SpectrumAnalyzerComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void SetSample(AudioSampleBuffer& sample);
private:
    Slider mPositionSlider;
    Label mPositionLabel;

    SimplePlot mPlot;
    SpectrumAnalyzer mSpectrumAnalyzer;

    //ntlab::Plot2D mPlot;
    std::unique_ptr<std::vector<OSC_Setting>> mMap;
    std::unique_ptr<AudioSampleBuffer> mSample;
    int sliderPosition{ 0 };

    void sliderValueChanged(Slider* slider) override;

    void Update();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerComponent)
};
