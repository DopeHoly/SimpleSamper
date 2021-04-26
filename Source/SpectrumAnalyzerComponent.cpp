/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 22 Mar 2021 6:59:06am
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SpectrumAnalyzerComponent.h"

//==============================================================================
SpectrumAnalyzerComponent::SpectrumAnalyzerComponent()
    :
    mSpectrumAnalyzer(44100, 58000)
    //mPlot(false, glContext)
{
    var sliderName = "Position";
    mPositionSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    mPositionSlider.setColour(mPositionSlider.backgroundColourId, Colours::transparentBlack);
    mPositionSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    mPositionSlider.setRange(0.0f, 5.0f, 0.01f);
    mPositionSlider.setName(sliderName);

    mPositionLabel.setFont(14.0f);
    mPositionLabel.setColour(mPositionLabel.textColourId, Colours::aqua);
    mPositionLabel.setText(sliderName, NotificationType::dontSendNotification);
    mPositionLabel.setJustificationType(Justification::centredTop);
    mPositionLabel.attachToComponent(&mPositionSlider, false);
    
    mPositionSlider.addListener(this);

    

    addAndMakeVisible(mPlot);
    addAndMakeVisible(mPositionSlider);
}

SpectrumAnalyzerComponent::~SpectrumAnalyzerComponent()
{
    mPositionSlider.removeListener(this);
}

void SpectrumAnalyzerComponent::paint (juce::Graphics& g)
{
}

void SpectrumAnalyzerComponent::resized()
{
    mPlot.setBoundsRelative(0, 0.25f, 1, 0.5f);
    mPositionSlider.setBoundsRelative(0.1f, 0.7f, 0.8f, 0.2f);

}

void SpectrumAnalyzerComponent::SetSample(AudioSampleBuffer& sample)
{
    mSample = std::make_unique<AudioSampleBuffer>(sample);
    auto numSamples = mSample->getNumSamples();
    mPositionSlider.setRange(0.0f, numSamples, mSpectrumAnalyzer.getWindowSize()/4/*N*/);
    mSpectrumAnalyzer.SetSample(sample);
    Update();
}

void SpectrumAnalyzerComponent::sliderValueChanged(Slider* slider)
{
    sliderPosition = slider->getValue();
    mSpectrumAnalyzer.SetPosition(sliderPosition);
    Update();
}

void SpectrumAnalyzerComponent::Update()
{
    mPlot.SetSample(mSpectrumAnalyzer.FFTSpectre().get());
    repaint();
}
