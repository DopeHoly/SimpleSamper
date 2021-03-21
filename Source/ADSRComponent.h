/*
  ==============================================================================

    ADSRComponent.h
    Created: 19 Mar 2021 1:51:38am
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class ADSRComponent  : public juce::Component
{
public:
    ADSRComponent(SimpleSamperAudioProcessor&);
    ~ADSRComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    SimpleSamperAudioProcessor& audioProcessor;

    Slider mAttackSlider, mDecaySlider, mSustainSlider, mReleaseSlider;
    Label mAttackLabel, mDecayLabel, mSustainLabel, mReleaseLabel;

    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mAttackAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mDecayAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mSustainAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mReleaseAttachment;

    void setMyStyleSlider(Slider& slider,
        std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>& attachment,
        String text,
        Label* label = nullptr
    );

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSRComponent)
};
