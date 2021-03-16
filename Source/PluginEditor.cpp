/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSamperAudioProcessorEditor::SimpleSamperAudioProcessorEditor (SimpleSamperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    mLoadButton.onClick = [&]() { audioProcessor.loadFileFromOpenFileDialog(); };
    addAndMakeVisible(mLoadButton);

    mLabel.setFont(15.0f);
    mLabel.setEditable(false);
    mLabel.setColour(Label::ColourIds::textColourId, Colours::white);
    addAndMakeVisible(mLabel);

    setSize (400, 300);
}

SimpleSamperAudioProcessorEditor::~SimpleSamperAudioProcessorEditor()
{
}

//==============================================================================
void SimpleSamperAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colours::black);

    if (audioProcessor.getNumSamplerSounds() > 0) {
        g.fillAll(Colours::green);
        mLabel.setText("Audio loaded", NotificationType::dontSendNotification);
    }
    else {
        mLabel.setText("Load a Sound", NotificationType::dontSendNotification);
        g.fillAll(Colours::black);
    }
}

void SimpleSamperAudioProcessorEditor::resized()
{
    //mLoadButton.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 50, 100, 100);
    mLabel.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 50, 100, 100);
}

bool SimpleSamperAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files) 
{
    WildcardFileFilter filter(audioProcessor.mFormatManager.getWildcardForAllFormats(), {}, {});

    for (String file : files) {
        if (!filter.isFileSuitable(file))
            return false;
    }
    return true;
}

void SimpleSamperAudioProcessorEditor::filesDropped(const StringArray& files, int x, int y) 
{
    for (auto file : files) {
        if(isInterestedInFileDrag(file)) {
            audioProcessor.loadFile(file);
        }
    }
    repaint();
}
