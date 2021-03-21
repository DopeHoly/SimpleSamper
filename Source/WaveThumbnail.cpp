/*
  ==============================================================================

    WaveThumbnail.cpp
    Created: 18 Mar 2021 10:47:51pm
    Author:  menin

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveThumbnail.h"

//==============================================================================
WaveThumbnail::WaveThumbnail(SimpleSamperAudioProcessor& processor)
    :audioProcessor(processor)
{
    //mWaveImage = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

WaveThumbnail::~WaveThumbnail()
{
}

void WaveThumbnail::paint (juce::Graphics& g)
{
    if (mShouldBePainting) {
        mWaveImage = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
        createWaveFormPicture(mWaveImage);
        mShouldBePainting = false;
    }

    g.drawImage(mWaveImage, Rectangle<float>(0, 0, getWidth(), getHeight()));

    drawVoicesPosition(g);
}

void WaveThumbnail::createWaveFormPicture(Image& image) 
{
    Graphics g(image);
    g.fillAll(Colours::aliceblue.darker());
    Path audioPath;
    mAudioPoints.clear();

    auto waveForm = audioProcessor.getWaveForm();
    auto numSamples = waveForm.getNumSamples();
    if (numSamples == 0) return;
    auto buffer = waveForm.getReadPointer(0);

    auto ratioX = numSamples / getWidth();
    auto ratioY = 2.5 / getHeight();

    //scale audio to window on X axis
    for (int sample = 0; sample < numSamples; sample += ratioX)
    {
        mAudioPoints.push_back(buffer[sample]);
    }

    audioPath.startNewSubPath(0, getHeight() / 2);

    //scale audio to window on Y axis
    for (int sample = 0; sample < mAudioPoints.size(); ++sample)
    {
        auto point = jmap<float>(mAudioPoints[sample], -1, 1, getHeight(), 0);
        audioPath.lineTo(sample, point);
    }
    g.strokePath(audioPath, PathStrokeType(2));
}


void WaveThumbnail::drawVoicesPosition(juce::Graphics& g)
{
    Synthesiser& sampler = audioProcessor.getSampler();
    int numVoices = sampler.getNumVoices();

    for (int i = 0; i < numVoices; ++i)
    {
        if (auto voice = dynamic_cast<MySamplerVoice*>(sampler.getVoice(i))) {
            drawVoicePosition(g, *voice);
        }
    }
}

void WaveThumbnail::drawVoicePosition(juce::Graphics& g, MySamplerVoice& voice)
{
    if (!voice.getADSR().isActive()) return;
    auto pos = voice.getSamplePosition();
    
    auto sound = static_cast<MySamplerSound*> (voice.getCurrentlyPlayingSound().get());

    if (sound == nullptr) return;

    auto numSamples = sound->getAudioData()->getNumSamples();

    auto posInMap = jmap<int>(pos, 0, numSamples, 0, getWidth());
    g.setColour(Colours::white);
    g.drawLine(posInMap, 0, posInMap, getHeight(), 2.0f);
}

void WaveThumbnail::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    mShouldBePainting = true;
}
