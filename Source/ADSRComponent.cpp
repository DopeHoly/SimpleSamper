/*
  ==============================================================================

    ADSRComponent.cpp
    Created: 19 Mar 2021 1:51:38am
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ADSRComponent.h"

//==============================================================================
ADSRComponent::ADSRComponent(SimpleSamperAudioProcessor& processor)
    :audioProcessor(processor)
{
    setMyStyleSlider(mAttackSlider, mAttackAttachment, "Attack", &mAttackLabel);
    setMyStyleSlider(mDecaySlider, mDecayAttachment, "Decay", &mDecayLabel);
    setMyStyleSlider(mSustainSlider, mSustainAttachment, "Sustain", &mSustainLabel);
    setMyStyleSlider(mReleaseSlider, mReleaseAttachment, "Release", &mReleaseLabel);

    addAndMakeVisible(mAttackSlider);
    addAndMakeVisible(mDecaySlider);
    addAndMakeVisible(mSustainSlider);
    addAndMakeVisible(mReleaseSlider);
}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint (juce::Graphics& g)
{
    g.fillAll(Colours::transparentBlack);
}

void ADSRComponent::resized()
{

    auto width = getWidth();
    auto height = getHeight();
    const int startX = 0;
    const int startY = height * 0.2;
    const int dialWidth = width / 4;
    const int dialHeight = height * 0.65f;
    const int stepX = dialWidth;

    mAttackSlider.setBounds(startX, startY, dialWidth, dialHeight);
    mDecaySlider.setBounds(startX + stepX, startY, dialWidth, dialHeight);
    mSustainSlider.setBounds(startX + stepX*2, startY, dialWidth, dialHeight);
    mReleaseSlider.setBounds(startX + stepX*3, startY, dialWidth, dialHeight);
}

void ADSRComponent::setMyStyleSlider(Slider& slider,
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>& attachment,
    String text,
    Label* label)
{
    slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    slider.setColour(slider.backgroundColourId, Colours::transparentBlack);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    slider.setRange(0.0f, 5.0f, 0.01f);
    slider.setName(text);

    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getAPVTS(),
        text.toUpperCase(),
        slider
        );

    if (label != nullptr) {
        label->setFont(14.0f);
        label->setColour(label->textColourId, Colours::aqua);
        label->setText(text, NotificationType::dontSendNotification);
        label->setJustificationType(Justification::centredTop);
        label->attachToComponent(&slider, false);
    }
}
