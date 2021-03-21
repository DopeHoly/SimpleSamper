/*
  ==============================================================================

    WaveThumbnail.h
    Created: 18 Mar 2021 10:47:51pm
    Author:  menin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class WaveThumbnail  : public juce::Component
{
public:
    WaveThumbnail(SimpleSamperAudioProcessor&);
    ~WaveThumbnail() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setShouldBePainting(bool value) { mShouldBePainting = value; }
    bool getShouldBePainting() { return mShouldBePainting; }
private:
    std::vector<float> mAudioPoints;
    bool mShouldBePainting{ true };

    Image mWaveImage;

    SimpleSamperAudioProcessor& audioProcessor;
    void createWaveFormPicture(Image& image);
    void drawVoicesPosition(juce::Graphics& g);
    void drawVoicePosition(juce::Graphics& g, MySamplerVoice&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveThumbnail)
};
